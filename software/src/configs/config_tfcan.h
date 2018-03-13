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

#define TFCAN_COM_LED_PIN                         P1_3
#define TFCAN_ERROR_LED_PIN                       P1_4

#define TFCAN_CLOCK_SOURCE                        XMC_CAN_CANCLKSRC_MCLK
#define TFCAN_FREQUENCY                           48000000

#define TFCAN_TX_PIN                              P0_4
#define TFCAN_TX_SRQ_INDEX                        0
#define TFCAN_TX_IRQ_INDEX                        0
#define TFCAN_TX_BUFFER_TIMEOUT_SETTLE_DURATION   2 // milliseconds

#define TFCAN_RX_PIN                              P0_5
#define TFCAN_RECEIVE_INPUT                       XMC_CAN_NODE_RECEIVE_INPUT_RXDCB
#define TFCAN_RX_SRQ_INDEX                        0
#define TFCAN_RX_IRQ_INDEX                        0

#define TFCAN_NODE_SIZE                           2

#define TFCAN_MO_SIZE                             32

#define TFCAN_RX_BUFFER_SIZE                      32

#define TFCAN_BACKLOG_SIZE                        768 // 13-byte frames

#endif
