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

static XMC_CAN_NODE_NOMINAL_BIT_TIME_CONFIG_t tfcan_bit_time_config = {
	.can_frequency = TFCAN_FREQUENCY,
	.baudrate      = 500000, // 500kbps, [100Kbps..1000Kbps]
	.sample_point  = 8000, // 80%, [0%..100%]
	.sjw           = 1, // [0..3]
};

static XMC_GPIO_CONFIG_t tfcan_rx_pin_config = {
	.mode             = XMC_GPIO_MODE_INPUT_TRISTATE,
	.input_hysteresis = XMC_GPIO_INPUT_HYSTERESIS_STANDARD,
};

const XMC_GPIO_CONFIG_t tfcan_tx_pin_config = {
	.mode         = XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT9,
	.output_level = XMC_GPIO_OUTPUT_LEVEL_HIGH
};

XMC_CAN_MO_t tfcan_mo0 = {
	.can_mo_ptr      = CAN_MO0,
	.can_priority    = XMC_CAN_ARBITRATION_MODE_ORDER_BASED_PRIO_1,
	.can_identifier  = 0xff,
	.can_id_mask     = 0xff,
	.can_id_mode     = XMC_CAN_FRAME_TYPE_STANDARD_11BITS,
	.can_ide_mask    = 1,
	.can_data_length = 8,
	.can_data        = {0x12345678, 0},
	.can_mo_type     = XMC_CAN_MO_TYPE_TRANSMSGOBJ
};

void tfcan_init(void) {
	memset(&tfcan, 0, sizeof(TFCAN));

	XMC_CAN_Init(CAN, TFCAN_CLOCK_SOURCE, TFCAN_FREQUENCY);

	// Configure bit timing
	XMC_CAN_NODE_NominalBitTimeConfigure(TFCAN_NODE, &tfcan_bit_time_config);

	XMC_CAN_NODE_EnableConfigurationChange(TFCAN_NODE);
	XMC_CAN_NODE_SetInitBit(TFCAN_NODE);

	// Configure RX pin
	XMC_GPIO_Init(TFCAN_RX_PIN, &tfcan_rx_pin_config);
	XMC_CAN_NODE_SetReceiveInput(TFCAN_NODE, TFCAN_RECEIVE_INPUT);

	// Configure TX pin
	XMC_GPIO_Init(TFCAN_TX_PIN, &tfcan_tx_pin_config);

	XMC_CAN_NODE_DisableConfigurationChange(TFCAN_NODE);
	XMC_CAN_NODE_ResetInitBit(TFCAN_NODE);

	// Configure MOs
	XMC_CAN_MO_Config(&tfcan_mo0);
	XMC_CAN_AllocateMOtoNodeList(CAN, TFCAN_NODE_NUM, 0);

	tfcan.last_transmit = system_timer_get_ms();
}

void tfcan_tick(void) {
	if (!system_timer_is_time_elapsed_ms(tfcan.last_transmit, 1000)) {
		return;
	}

	++tfcan_mo0.can_data[1];

	logi("update %d\n\r", XMC_CAN_MO_UpdateData(&tfcan_mo0));
	logi("transmit %d\n\r", XMC_CAN_MO_Transmit(&tfcan_mo0));

	tfcan.last_transmit = system_timer_get_ms();
}
