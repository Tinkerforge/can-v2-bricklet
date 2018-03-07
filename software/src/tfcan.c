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
	memset(&tfcan, 0, sizeof(TFCAN));

	tfcan.tx_mo_count = TFCAN_MO_COUNT / 2;
	tfcan.tx_mo_next_index = 0;

	tfcan.rx_mo_count = TFCAN_MO_COUNT / 2;
	tfcan.rx_mo_next_index = 0;

	tfcan.tx_buffer = tfcan.buffer;
	tfcan.tx_buffer_size = TFCAN_BUFFER_SIZE / 2;
	tfcan.tx_buffer_start = 0;
	tfcan.tx_buffer_end = 0;

	tfcan.rx_buffer = &tfcan.buffer[tfcan.tx_buffer_size];
	tfcan.rx_buffer_size = TFCAN_BUFFER_SIZE / 2;
	tfcan.rx_buffer_start = 0;
	tfcan.rx_buffer_end = 0;

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

	tfcan_reconfigure_mos();

	tfcan.last_transmit = system_timer_get_ms();
}

void tfcan_tick(void) {
	/*if (!system_timer_is_time_elapsed_ms(tfcan.last_transmit, 1000)) {
		return;
	}

	for (uint8_t i = 0; i < tfcan.tx_mo_count; ++i) {
		logi("st-tx %02d %032_8b\n\r", i, tfcan_mo_get_status(tfcan.tx_mo[i]));
	}

	logi("fgpr-tx %032_8b\n\r", tfcan.tx_mo[0]->MOFGPR);

	for (uint8_t i = 0; i < tfcan.rx_mo_count; ++i) {
		logi("st-rx %02d %032_8b\n\r", i, tfcan_mo_get_status(tfcan.rx_mo[i]));
	}

	logi("fgpr-rx %032_8b\n\r", tfcan.rx_mo[0]->MOFGPR);*/

	// write at most TX FIFO size frames
	for (uint8_t i = 0; i < tfcan.tx_mo_count; ++i) {
		// check TX buffer
		if (tfcan.tx_buffer_start == tfcan.tx_buffer_end) {
			break; // TX buffer empty
		}

		// check TX FIFO
		CAN_MO_TypeDef *tx_mo_next = tfcan.tx_mo[tfcan.tx_mo_next_index];
		uint32_t status = tfcan_mo_get_status(tx_mo_next);

		if ((status & TFCAN_MO_STATUS_TX_REQUEST) != 0) {
			break; // TX FIFO full
		}

		logi("tx %d\n\r", tfcan.tx_mo_next_index);

		// copy frame into MO
		TFCAN_Frame *frame = &tfcan.tx_buffer[tfcan.tx_buffer_start];

		tfcan.tx_buffer_start = (tfcan.tx_buffer_start + 1) % tfcan.tx_buffer_size;

		tfcan_mo_set_identifier(tx_mo_next, frame->type, frame->identifier);
		tfcan_mo_set_data(tx_mo_next, frame->data, frame->length);

		// schedule MO for transmission
		tfcan_mo_change_status(tx_mo_next, TFCAN_MO_SET_STATUS_TX_REQUEST);

		tfcan.tx_mo_next_index = (tfcan.tx_mo_next_index + 1) % tfcan.tx_mo_count;
	}

	// read at most RX FIFO size frames
	for (uint8_t i = 0; i < tfcan.rx_mo_count; ++i) {
		// check RX buffer
		if ((tfcan.rx_buffer_end + 1) % tfcan.rx_buffer_size == tfcan.rx_buffer_start) {
			// FIXME: drop RX MO?

			break; // RX buffer full
		}

		// check RX FIFO
		CAN_MO_TypeDef *rx_mo_next = tfcan.rx_mo[tfcan.rx_mo_next_index];
		uint32_t status = tfcan_mo_get_status(rx_mo_next);

		if ((status & TFCAN_MO_STATUS_RX_PENDING) == 0) {
			break; // RX FIFO empty
		}

		logi("rx %d\n\r", tfcan.rx_mo_next_index);

		TFCAN_Frame *frame = &tfcan.rx_buffer[tfcan.rx_buffer_end];

		tfcan.rx_buffer_end = (tfcan.rx_buffer_end + 1) % tfcan.rx_buffer_size;

		do {
			tfcan_mo_change_status(rx_mo_next, TFCAN_MO_RESET_STATUS_NEW_DATA |
			                                   TFCAN_MO_RESET_STATUS_RX_PENDING);

			// copy MO to frame
			TFCAN_MOType type;
			uint32_t identifier;

			tfcan_mo_get_identifier(rx_mo_next, &type, &identifier);

			frame->type = type;
			frame->identifier = identifier;

			tfcan_mo_get_data(rx_mo_next, frame->data, &frame->length);

			logi("rx try %d\n\r", tfcan.rx_mo_next_index);

			status = tfcan_mo_get_status(rx_mo_next);
		} while ((status & TFCAN_MO_STATUS_NEW_DATA) != 0 ||
		         (status & TFCAN_MO_STATUS_RX_UPDATING) != 0); // FIXME: add timeout

		tfcan.rx_mo_next_index = (tfcan.rx_mo_next_index + 1) % tfcan.rx_mo_count;
	}

	tfcan.last_transmit = system_timer_get_ms();
}

void tfcan_reconfigure_mos(void) {
	// reset MOs
	XMC_CAN_NODE_EnableConfigurationChange(TFCAN_NODE);
	XMC_CAN_NODE_SetInitBit(TFCAN_NODE);

	while (!XMC_CAN_IsPanelControlReady(CAN)) {}
	XMC_CAN_PanelControl(CAN, XMC_CAN_PANCMD_INIT_LIST, 0, 0);
	while (!XMC_CAN_IsPanelControlReady(CAN)) {}

	XMC_CAN_NODE_DisableConfigurationChange(TFCAN_NODE);
	XMC_CAN_NODE_ResetInitBit(TFCAN_NODE);

	// configure TX MOs
	for (uint8_t i = 0; i < tfcan.tx_mo_count; ++i) {
		tfcan.tx_mo[i] = (CAN_MO_TypeDef *)&CAN_MO->MO[i];

		XMC_CAN_AllocateMOtoNodeList(CAN, TFCAN_NODE_INDEX, i);
		while (!XMC_CAN_IsPanelControlReady(CAN)) {}

		tfcan_mo_init_tx(tfcan.tx_mo[i]);
		tfcan_mo_set_irq_pointer(tfcan.tx_mo[i], TFCAN_MO_IRQ_POINTER_TRANSMIT, TFCAN_TX_SRQ_INDEX);
		tfcan_mo_enable_event(tfcan.tx_mo[i], TFCAN_MO_EVENT_TRANSMIT);
	}

	// configure TX FIFO
	uint8_t tx_fifo_base = 0; // also bottom

	tfcan_mo_init_tx_fifo_base(tfcan.tx_mo[0], tx_fifo_base, tx_fifo_base + tfcan.tx_mo_count - 1);

	for (uint8_t i = 1; i < tfcan.tx_mo_count; ++i) {
		tfcan_mo_init_tx_fifo_slave(tfcan.tx_mo[i], tx_fifo_base);
	}

	// configure RX MOs
	for (uint8_t i = 0; i < tfcan.rx_mo_count; ++i) {
		tfcan.rx_mo[i] = (CAN_MO_TypeDef *)&CAN_MO->MO[tfcan.tx_mo_count + i];

		XMC_CAN_AllocateMOtoNodeList(CAN, TFCAN_NODE_INDEX, tfcan.tx_mo_count + i);
		while (!XMC_CAN_IsPanelControlReady(CAN)) {}

		tfcan_mo_init_rx(tfcan.rx_mo[i]);
		tfcan_mo_set_irq_pointer(tfcan.rx_mo[i], TFCAN_MO_IRQ_POINTER_RECEIVE, TFCAN_RX_SRQ_INDEX);
		tfcan_mo_enable_event(tfcan.rx_mo[i], TFCAN_MO_EVENT_RECEIVE);
	}

	// configure TX FIFO
	uint8_t rx_fifo_base = tfcan.tx_mo_count;

	tfcan_mo_init_rx_fifo_base(tfcan.rx_mo[0], rx_fifo_base, rx_fifo_base + tfcan.rx_mo_count - 1);

	for (uint8_t i = 1; i < tfcan.rx_mo_count; ++i) {
		tfcan_mo_init_rx_fifo_slave(tfcan.rx_mo[i], rx_fifo_base);
	}
}

// add frame to TX buffer
bool tfcan_enqueue_frame(TFCAN_Frame *frame) {
	uint16_t new_tx_buffer_end = (tfcan.tx_buffer_end + 1) % tfcan.tx_buffer_size;

	if (new_tx_buffer_end == tfcan.tx_buffer_start) {
		return false; // TX buffer full
	}

	memcpy(&tfcan.tx_buffer[tfcan.tx_buffer_end], frame, sizeof(TFCAN_Frame));
	tfcan.tx_buffer_end = new_tx_buffer_end;

	return true;
}

// remove frame from RX buffer
bool tfcan_dequeue_frame(TFCAN_Frame *frame) {
	if (tfcan.rx_buffer_start == tfcan.rx_buffer_end) {
		return false; // RX buffer empty
	}

	memcpy(frame, &tfcan.rx_buffer[tfcan.rx_buffer_start], sizeof(TFCAN_Frame));
	tfcan.rx_buffer_start = (tfcan.rx_buffer_start + 1) % tfcan.tx_buffer_size;

	return true;
}
