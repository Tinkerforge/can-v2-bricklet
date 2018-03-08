/* can-v2-bricklet
 * Copyright (C) 2018 Matthias Bolte <matthias@tinkerforge.com>
 *
 * tfcan.h: Transfer data over CAN bus
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

#ifndef TFCAN_H
#define TFCAN_H

#include <stdint.h>
#include <stdbool.h>

#include "configs/config_tfcan.h"

#include "xmc_can.h"

#include "tfcan_mo.h"

typedef struct {
	struct {
		uint32_t mo_type:3; // TFCAN_MOType
		uint32_t identifier:29;
	};
	uint8_t data[8];
	uint8_t length;
} __attribute__((__packed__)) TFCAN_Frame; // 13 bytes

typedef struct {
	CAN_MO_TypeDef *tx_mo[TFCAN_MO_SIZE];
	uint8_t tx_mo_size; // [1..TFCAN_MO_SIZE-1]
	uint8_t tx_mo_next_index;

	CAN_MO_TypeDef *rx_mo[TFCAN_MO_SIZE];
	uint8_t rx_mo_size; // [1..TFCAN_MO_SIZE-1]
	uint8_t rx_mo_next_index;

	TFCAN_Frame backlog[TFCAN_BACKLOG_SIZE];

	TFCAN_Frame *tx_backlog;
	uint16_t tx_backlog_size; // [1..TFCAN_BACKLOG_SIZE-1]
	uint16_t tx_backlog_start;
	uint16_t tx_backlog_end;

	TFCAN_Frame *rx_backlog;
	uint16_t rx_backlog_size; // [1..TFCAN_BACKLOG_SIZE-1]
	uint16_t rx_backlog_start;
	uint16_t rx_backlog_end;

	uint32_t last_transmit;
} TFCAN; // avoid name collision with global XMC CAN object named CAN

void tfcan_init(void);
void tfcan_tick(void);

void tfcan_reconfigure_mos(void);

bool tfcan_enqueue_frame(TFCAN_Frame *frame);
bool tfcan_dequeue_frame(TFCAN_Frame *frame);

#endif
