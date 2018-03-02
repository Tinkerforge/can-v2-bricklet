/* can-v2-bricklet
 * Copyright (C) 2018 Matthias Bolte <matthias@tinkerforge.com>
 *
 * config_tfcan.h: CAN specific configuration
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

#ifndef CONFIG_TFCAN_H
#define CONFIG_TFCAN_H

#include "xmc_gpio.h"
#include "xmc_can.h"
#include "xmc_can_map.h"

#define TFCAN_CLOCK_SOURCE               XMC_CAN_CANCLKSRC_MCLK
#define TFCAN_FREQUENCY                  48000000
#define TFCAN_NODE                       CAN_NODE0
#define TFCAN_NODE_NUM                   0
#define TFCAN_RX_PIN                     P0_5
#define TFCAN_RECEIVE_INPUT              XMC_CAN_NODE_RECEIVE_INPUT_RXDCB
#define TFCAN_TX_PIN                     P0_4



/*
#define PCF85263A_I2C_BAUDRATE         100000

#define PCF85263A_I2C_ADDRESS          0b1010001
#define PCF85263A_I2C                  XMC_I2C0_CH1

#define PCF85263A_SCL_PORT             XMC_GPIO_PORT0
#define PCF85263A_SCL_PIN              8
#define PCF85263A_SCL_PIN_MODE         XMC_GPIO_MODE_OUTPUT_OPEN_DRAIN_ALT7
#define PCF85263A_SCL_INPUT            XMC_USIC_CH_INPUT_DX1
#define PCF85263A_SCL_SOURCE           1
#define PCF85263A_SCL_FIFO_SIZE        XMC_USIC_CH_FIFO_SIZE_16WORDS
#define PCF85263A_SCL_FIFO_POINTER     32

#define PCF85263A_SDA_PORT             XMC_GPIO_PORT0
#define PCF85263A_SDA_PIN              6
#define PCF85263A_SDA_PIN_MODE         XMC_GPIO_MODE_OUTPUT_OPEN_DRAIN_ALT7
#define PCF85263A_SDA_INPUT            XMC_USIC_CH_INPUT_DX0
#define PCF85263A_SDA_SOURCE           2
#define PCF85263A_SDA_FIFO_SIZE        XMC_USIC_CH_FIFO_SIZE_16WORDS
#define PCF85263A_SDA_FIFO_POINTER     48

#define PCF85263A_INT_PIN              P0_9*/

#endif
