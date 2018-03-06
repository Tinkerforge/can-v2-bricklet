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
	tfcan.rx_mo_count = TFCAN_MO_COUNT / 2;

	tfcan.next_tx_mo_index = 0;

	tfcan.tx_buffer_size = TFCAN_BUFFER_SIZE / 2;
	tfcan.tx_buffer_start = 0;
	tfcan.tx_buffer_end = 0;
	tfcan.rx_buffer_size = TFCAN_BUFFER_SIZE / 2;
	tfcan.rx_buffer_start = 0;
	tfcan.rx_buffer_end = 0;

	// configure clock
	XMC_CAN_Init(CAN, TFCAN_CLOCK_SOURCE, TFCAN_FREQUENCY);

	// configure bit timing
	const XMC_CAN_NODE_NOMINAL_BIT_TIME_CONFIG_t bit_time_config = {
		.can_frequency = TFCAN_FREQUENCY,
		.baudrate      = 500000, // 500kbps, [100Kbps..1000Kbps]
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

	tfcan_reconfigure_mo();

	tfcan.last_transmit = system_timer_get_ms();
	tfcan.next_fake_data = 0;
}

void tfcan_tick(void) {
	if (!system_timer_is_time_elapsed_ms(tfcan.last_transmit, 1000)) {
		return;
	}

	for (uint8_t i = 0; i < tfcan.tx_mo_count; ++i) {
		logi("st %d %032_8b\n\r", i, XMC_CAN_MO_GetStatus(&tfcan.mo[i]));
	}

	logi("fgpr %032_8b\n\r", tfcan.mo[0].can_mo_ptr->MOFGPR);

	for (uint8_t k = 0; k < 3; ++k) {
		XMC_CAN_MO_t *next_tx_mo = &tfcan.mo[tfcan.next_tx_mo_index];
		uint32_t next_tx_mo_status = XMC_CAN_MO_GetStatus(next_tx_mo);

		if ((next_tx_mo_status & XMC_CAN_MO_STATUS_TX_REQUEST) == 0) {
			logi("tx %d\n\r", tfcan.next_tx_mo_index);

			//XMC_CAN_MO_SetStandardID(next_tx_mo);
			XMC_CAN_MO_SetExtendedID(next_tx_mo);

			XMC_CAN_MO_SetIdentifier(next_tx_mo, (0b11111111111 << 18) | tfcan.next_fake_data);

			next_tx_mo->can_data_byte[0] = tfcan.next_fake_data;
			next_tx_mo->can_data_length = 1;

			XMC_CAN_MO_UpdateData(next_tx_mo);

			XMC_CAN_MO_SetStatus(next_tx_mo, XMC_CAN_MO_SET_STATUS_TX_REQUEST);

			++tfcan.next_fake_data;
			tfcan.next_tx_mo_index = (tfcan.next_tx_mo_index + 1) % tfcan.tx_mo_count;
		}
	}

	tfcan.last_transmit = system_timer_get_ms();
}

void tfcan_reconfigure_mo(void) {
	// reset MOs
	XMC_CAN_NODE_EnableConfigurationChange(TFCAN_NODE);
	XMC_CAN_NODE_SetInitBit(TFCAN_NODE);

	while (!XMC_CAN_IsPanelControlReady(CAN)) {}
	XMC_CAN_PanelControl(CAN, XMC_CAN_PANCMD_INIT_LIST, 0, 0);
	while (!XMC_CAN_IsPanelControlReady(CAN)) {}

	XMC_CAN_NODE_DisableConfigurationChange(TFCAN_NODE);
	XMC_CAN_NODE_ResetInitBit(TFCAN_NODE);

	// clear MO wrappers
	memset(tfcan.mo, 0, sizeof(XMC_CAN_MO_t) * TFCAN_MO_COUNT);

	// configure TX MOs
	for (uint8_t i = 0; i < tfcan.tx_mo_count; ++i) {
		tfcan.mo[i].can_mo_ptr   = (CAN_MO_TypeDef *)&CAN_MO->MO[i];
		tfcan.mo[i].can_priority = XMC_CAN_ARBITRATION_MODE_ORDER_BASED_PRIO_1;
		tfcan.mo[i].can_mo_type  = XMC_CAN_MO_TYPE_TRANSMSGOBJ;

		XMC_CAN_AllocateMOtoNodeList(CAN, TFCAN_NODE_INDEX, i);
		while (!XMC_CAN_IsPanelControlReady(CAN)) {}

		XMC_CAN_MO_Config(&tfcan.mo[i]);
		XMC_CAN_MO_SetEventNodePointer(&tfcan.mo[i], XMC_CAN_MO_POINTER_EVENT_TRANSMIT, TFCAN_TX_SRQ_INDEX);
		XMC_CAN_MO_EnableEvent(&tfcan.mo[i], XMC_CAN_MO_EVENT_TRANSMIT);

		// clear RXEN that is set by XMC_CAN_MO_Config even for transmit MOs
		XMC_CAN_MO_SetStatus(&tfcan.mo[i], XMC_CAN_MO_RESET_STATUS_RX_ENABLE);
	}

	// configure TX FIFO
	const XMC_CAN_FIFO_CONFIG_t tx_fifo = {
		.fifo_bottom = 0,
		.fifo_top    = tfcan.tx_mo_count - 1,
		.fifo_base   = 0
	};

	XMC_CAN_TXFIFO_ConfigMOBaseObject(&tfcan.mo[0], tx_fifo);

	for (uint8_t i = 1; i < tfcan.tx_mo_count; ++i) {
		XMC_CAN_TXFIFO_ConfigMOSlaveObject(&tfcan.mo[i], tx_fifo);
	}
}

bool tfcan_write_frame(TFCANFrame *frame) {
	return false;
}

bool tfcan_read_frame(TFCANFrame *frame) {
	return false;
}
