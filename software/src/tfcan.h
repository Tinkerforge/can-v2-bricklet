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

typedef enum {
	TFCAN_FRAME_TYPE_STANDARD_DATA = 0,
	TFCAN_FRAME_TYPE_STANDARD_REMOTE,
	TFCAN_FRAME_TYPE_EXTENDED_DATA,
	TFCAN_FRAME_TYPE_EXTENDED_REMOTE,
} TFCANFrameType;

typedef struct {
	struct {
		uint32_t type:3;
		uint32_t identifier:29;
	};
	uint8_t data[8];
	uint8_t length;
} __attribute__((__packed__)) TFCANFrame; // 13 bytes

typedef struct {
	XMC_CAN_MO_t mo[TFCAN_MO_COUNT];

	uint8_t tx_mo_count; // [1..TFCAN_MO_COUNT-1]
	uint8_t rx_mo_count; // [1..TFCAN_MO_COUNT-1]

	uint8_t next_tx_mo_index;

	TFCANFrame buffer[TFCAN_BUFFER_SIZE];
	uint16_t tx_buffer_size; // [1..TFCAN_BUFFER_SIZE-1]
	uint16_t tx_buffer_start;
	uint16_t tx_buffer_end;
	uint16_t rx_buffer_size; // [1..TFCAN_BUFFER_SIZE-1]
	uint16_t rx_buffer_start;
	uint16_t rx_buffer_end;

	uint32_t last_transmit;
	uint8_t next_fake_data;
} TFCAN; // Avoid name collision with global XMC CAN object named CAN

void tfcan_init(void);
void tfcan_tick(void);

void tfcan_reconfigure_mo(void);

bool tfcan_write_frame(TFCANFrame *frame);
bool tfcan_read_frame(TFCANFrame *frame);

#endif
