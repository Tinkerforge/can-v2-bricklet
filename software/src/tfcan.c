/* can-v2-bricklet
 * Copyright (C) 2018 Matthias Bolte <matthias@tinkerforge.com>
 *
 * tfcan.c: Transfer data over CAN bus
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "tfcan.h"

#include "configs/config.h"
#include "configs/config_tfcan.h"
#include "bricklib2/hal/system_timer/system_timer.h"
#include "bricklib2/logging/logging.h"

#include "xmc_gpio.h"
#include "xmc_can.h"
#include "xmc_can_map.h"

TFCAN tfcan;

void IRQ_Hdlr_0(void) {
	//logi("irq0: %b, %d\n\r", CAN->MSPND[0], XMC_CAN_FIFO_GetCurrentMO(&tfcan.mo[0]));
}

void tfcan_init(void) {
	logi("TFCAN %u\n\r", sizeof(TFCAN));

	memset(&tfcan, 0, sizeof(TFCAN));

	// configure clock
	XMC_CAN_Init(CAN, TFCAN_CLOCK_SOURCE, TFCAN_FREQUENCY);

	// configure bit timing
	const XMC_CAN_NODE_NOMINAL_BIT_TIME_CONFIG_t bit_time_config = {
		.can_frequency = TFCAN_FREQUENCY,
		.baudrate      = 125000, // 500kbps, [100Kbps..1000Kbps]
		.sample_point  = 8000, // 80%, [0%..100%]
		.sjw           = 1, // [1..4]
	};

	XMC_CAN_NODE_NominalBitTimeConfigure(TFCAN_NODE, &bit_time_config);

	XMC_CAN_NODE_EnableConfigurationChange(TFCAN_NODE);
	XMC_CAN_NODE_SetInitBit(TFCAN_NODE);

	// configure RX pin
	const XMC_GPIO_CONFIG_t rx_pin_config = {
		.mode             = XMC_GPIO_MODE_INPUT_TRISTATE,
		.input_hysteresis = XMC_GPIO_INPUT_HYSTERESIS_STANDARD,
	};

	XMC_GPIO_Init(TFCAN_RX_PIN, &rx_pin_config);
	XMC_CAN_NODE_SetReceiveInput(TFCAN_NODE, TFCAN_RECEIVE_INPUT);

	// configure TX pin
	const XMC_GPIO_CONFIG_t tx_pin_config = {
		.mode         = XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT9,
		.output_level = XMC_GPIO_OUTPUT_LEVEL_HIGH
	};

	XMC_GPIO_Init(TFCAN_TX_PIN, &tx_pin_config);

	XMC_CAN_NODE_DisableConfigurationChange(TFCAN_NODE);
	XMC_CAN_NODE_ResetInitBit(TFCAN_NODE);

	// configure TX interrupt
	//NVIC_EnableIRQ(TFCAN_TX_IRQ_INDEX);
	//NVIC_SetPriority(TFCAN_TX_IRQ_INDEX, 0);
	//XMC_SCU_SetInterruptControl(TFCAN_TX_IRQ_INDEX, XMC_SCU_IRQCTRL_CAN0_SR0_IRQ0);

	tfcan_reconfigure_queues();

	// configure extra LEDs
	XMC_GPIO_CONFIG_t led_pin_config = {
		.mode         = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
		.output_level = XMC_GPIO_OUTPUT_LEVEL_HIGH
	};

	XMC_GPIO_Init(TFCAN_COM_LED_PIN, &led_pin_config);
	XMC_GPIO_Init(TFCAN_ERROR_LED_PIN, &led_pin_config);

	tfcan.last_debug = system_timer_get_ms();
}

void tfcan_tick(void) {
	/*if (system_timer_is_time_elapsed_ms(tfcan.last_debug, 1000)) {
		for (uint8_t i = 0; i < tfcan.tx_mo_size; ++i) {
			logi("st-tx %02d %032_8b\n\r", i, tfcan_mo_get_status(tfcan.tx_mo[i]));
		}

		logi("fgpr-tx %032_8b\n\r", tfcan.tx_mo[0]->MOFGPR);

		for (uint8_t k = 0; k < TFCAN_MO_SIZE; ++k) {
			for (uint8_t i = 0; i < tfcan.rx_mo_size[k]; ++i) {
				logi("st-rx %02d:%02d %032_8b\n\r", k, i, tfcan_mo_get_status(tfcan.rx_mo[k][i]));
			}

			if (tfcan.rx_mo_size[k] > 0) {
				logi("fgpr-rx %02d %032_8b\n\r", k, tfcan.rx_mo[k][0]->MOFGPR);
			}
		}

		tfcan.last_debug = system_timer_get_ms();
	}*/

	// write at most TX FIFO size frames
	for (uint8_t i = 0; i < tfcan.tx_mo_size; ++i) {
		// check TX backlog
		if (tfcan.tx_backlog_start == tfcan.tx_backlog_end) {
			break; // TX backlog empty
		}

		// check TX FIFO
		CAN_MO_TypeDef *tx_mo_next = tfcan.tx_mo[tfcan.tx_mo_next_index];
		uint32_t status = tfcan_mo_get_status(tx_mo_next);

		if ((status & TFCAN_MO_STATUS_TX_REQUEST) != 0) {
			break; // TX FIFO full
		}

		logi("tx %d\n\r", tfcan.tx_mo_next_index);

		// copy frame into MO
		TFCAN_Frame *frame = &tfcan.tx_backlog[tfcan.tx_backlog_start];

		tfcan.tx_backlog_start = (tfcan.tx_backlog_start + 1) % tfcan.tx_backlog_size;

		tfcan_mo_set_identifier(tx_mo_next, frame->mo_type, frame->identifier);
		tfcan_mo_set_data(tx_mo_next, frame->data, frame->length);

		// schedule MO for transmission
		tfcan_mo_change_status(tx_mo_next, TFCAN_MO_SET_STATUS_TX_REQUEST);

		tfcan.tx_mo_next_index = (tfcan.tx_mo_next_index + 1) % tfcan.tx_mo_size;
	}

	// FIXME: need to read from RX FIFOs ordered by reception time
	for (uint8_t k = 0; k < TFCAN_MO_SIZE; ++k) {
		CAN_MO_TypeDef **rx_mo = tfcan.rx_mo[k];
		const uint8_t rx_mo_size = tfcan.rx_mo_size[k];
		uint8_t rx_mo_next_index = tfcan.rx_mo_next_index[k];

		// read at most RX FIFO size frames
		for (uint8_t i = 0; i < rx_mo_size; ++i) {
			// check RX backlog
			if ((tfcan.rx_backlog_end + 1) % tfcan.rx_backlog_size == tfcan.rx_backlog_start) {
				// FIXME: drop RX MO?

				break; // RX backlog full
			}

			// check RX FIFO
			CAN_MO_TypeDef *rx_mo_next = rx_mo[rx_mo_next_index];
			uint32_t status = tfcan_mo_get_status(rx_mo_next);

			if ((status & TFCAN_MO_STATUS_RX_PENDING) == 0) {
				break; // RX FIFO empty
			}

			logi("rx %d:%d\n\r", k, rx_mo_next_index);

			TFCAN_Frame *frame = &tfcan.rx_backlog[tfcan.rx_backlog_end];

			tfcan.rx_backlog_end = (tfcan.rx_backlog_end + 1) % tfcan.rx_backlog_size;

			do {
				tfcan_mo_change_status(rx_mo_next, TFCAN_MO_RESET_STATUS_NEW_DATA |
				                                   TFCAN_MO_RESET_STATUS_RX_PENDING);

				// copy MO to frame
				TFCAN_MOType mo_type;
				uint32_t identifier;

				tfcan_mo_get_identifier(rx_mo_next, &mo_type, &identifier);

				frame->mo_type = mo_type;
				frame->identifier = identifier;

				tfcan_mo_get_data(rx_mo_next, frame->data, &frame->length);

				logi("rx %d:%d try\n\r", k, rx_mo_next_index);

				status = tfcan_mo_get_status(rx_mo_next);
			} while ((status & TFCAN_MO_STATUS_NEW_DATA) != 0 ||
			         (status & TFCAN_MO_STATUS_RX_UPDATING) != 0); // FIXME: add timeout

			rx_mo_next_index = (rx_mo_next_index + 1) % rx_mo_size;
		}

		tfcan.rx_mo_next_index[k] = rx_mo_next_index;
	}
}

void tfcan_reconfigure_queues(void) {
	// reset queues
	tfcan.tx_mo_size = TFCAN_MO_SIZE / 2;
	tfcan.tx_mo_next_index = 0;

	tfcan.rx_mo_size[0] = TFCAN_MO_SIZE / 4;
	tfcan.rx_mo_next_index[0] = 0;
	tfcan.rx_mo_type[0] = TFCAN_BUFFER_TYPE_DATA;

	tfcan.rx_mo_size[1] = TFCAN_MO_SIZE / 4;
	tfcan.rx_mo_next_index[1] = 0;
	tfcan.rx_mo_type[1] = TFCAN_BUFFER_TYPE_REMOTE;

	tfcan.tx_backlog = tfcan.backlog;
	tfcan.tx_backlog_size = TFCAN_BACKLOG_SIZE / 2;
	tfcan.tx_backlog_start = 0;
	tfcan.tx_backlog_end = 0;

	tfcan.rx_backlog = &tfcan.backlog[tfcan.tx_backlog_size];
	tfcan.rx_backlog_size = TFCAN_BACKLOG_SIZE / 2;
	tfcan.rx_backlog_start = 0;
	tfcan.rx_backlog_end = 0;

	// reset MOs
	XMC_CAN_NODE_EnableConfigurationChange(TFCAN_NODE);
	XMC_CAN_NODE_SetInitBit(TFCAN_NODE);

	while (!XMC_CAN_IsPanelControlReady(CAN)) {}
	XMC_CAN_PanelControl(CAN, XMC_CAN_PANCMD_INIT_LIST, 0, 0);
	while (!XMC_CAN_IsPanelControlReady(CAN)) {}

	XMC_CAN_NODE_DisableConfigurationChange(TFCAN_NODE);
	XMC_CAN_NODE_ResetInitBit(TFCAN_NODE);

	// configure TX MOs
	uint8_t mo_offset = 0;

	tfcan.tx_mo = &tfcan.mo[mo_offset];

	for (uint8_t i = 0; i < tfcan.tx_mo_size; ++i) {
		tfcan.tx_mo[i] = (CAN_MO_TypeDef *)&CAN_MO->MO[mo_offset + i];

		XMC_CAN_AllocateMOtoNodeList(CAN, TFCAN_NODE_INDEX, mo_offset + i);
		while (!XMC_CAN_IsPanelControlReady(CAN)) {}

		tfcan_mo_init_tx(tfcan.tx_mo[i]);
		tfcan_mo_set_irq_pointer(tfcan.tx_mo[i], TFCAN_MO_IRQ_POINTER_TRANSMIT, TFCAN_TX_SRQ_INDEX);
		tfcan_mo_enable_event(tfcan.tx_mo[i], TFCAN_MO_EVENT_TRANSMIT);
	}

	// configure TX FIFO
	if (tfcan.tx_mo_size > 0) {
		tfcan_mo_init_tx_fifo_base(tfcan.tx_mo[0], mo_offset, mo_offset + tfcan.tx_mo_size - 1);

		for (uint8_t i = 1; i < tfcan.tx_mo_size; ++i) {
			tfcan_mo_init_tx_fifo_slave(tfcan.tx_mo[i], mo_offset);
		}
	}

	// configure RX MOs and FIFOs
	mo_offset += tfcan.tx_mo_size;

	for (uint8_t k = 0; k < TFCAN_MO_SIZE; ++k) {
		tfcan.rx_mo[k] = &tfcan.mo[mo_offset];

		CAN_MO_TypeDef **rx_mo = tfcan.rx_mo[k];
		const uint8_t rx_mo_size = tfcan.rx_mo_size[k];
		const TFCAN_BufferType rx_mo_type = tfcan.rx_mo_type[k];

		// configure RX MOs
		for (uint8_t i = 0; i < rx_mo_size; ++i) {
			rx_mo[i] = (CAN_MO_TypeDef *)&CAN_MO->MO[mo_offset + i];

			XMC_CAN_AllocateMOtoNodeList(CAN, TFCAN_NODE_INDEX, mo_offset + i);
			while (!XMC_CAN_IsPanelControlReady(CAN)) {}

			tfcan_mo_init_rx(rx_mo[i], rx_mo_type);
			tfcan_mo_set_irq_pointer(rx_mo[i], TFCAN_MO_IRQ_POINTER_RECEIVE, TFCAN_RX_SRQ_INDEX);
			tfcan_mo_enable_event(rx_mo[i], TFCAN_MO_EVENT_RECEIVE);
		}

		// configure RX FIFO
		if (rx_mo_size > 0) {
			tfcan_mo_init_rx_fifo_base(rx_mo[0], mo_offset, mo_offset + rx_mo_size - 1);

			for (uint8_t i = 1; i < rx_mo_size; ++i) {
				tfcan_mo_init_rx_fifo_slave(rx_mo[i], mo_offset);
			}
		}

		mo_offset += rx_mo_size;
	}
}

// add frame to TX backlog
bool tfcan_enqueue_frame(TFCAN_Frame *frame) {
	uint16_t new_tx_backlog_end = (tfcan.tx_backlog_end + 1) % tfcan.tx_backlog_size;

	if (new_tx_backlog_end == tfcan.tx_backlog_start) {
		return false; // TX backlog full
	}

	memcpy(&tfcan.tx_backlog[tfcan.tx_backlog_end], frame, sizeof(TFCAN_Frame));
	tfcan.tx_backlog_end = new_tx_backlog_end;

	return true;
}

// remove frame from RX backlog
bool tfcan_dequeue_frame(TFCAN_Frame *frame) {
	if (tfcan.rx_backlog_start == tfcan.rx_backlog_end) {
		return false; // RX backlog empty
	}

	memcpy(frame, &tfcan.rx_backlog[tfcan.rx_backlog_start], sizeof(TFCAN_Frame));
	tfcan.rx_backlog_start = (tfcan.rx_backlog_start + 1) % tfcan.tx_backlog_size;

	return true;
}
