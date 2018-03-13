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

// must match Bricklet API TransceiverMode enum
typedef enum {
	TFCAN_TRANSCEIVER_MODE_NORMAL = 0,
	TFCAN_TRANSCEIVER_MODE_LOOPBACK,
	TFCAN_TRANSCEIVER_MODE_READ_ONLY
} TFCAN_TransceiverMode;

typedef struct {
	struct {
		uint32_t mo_type:3; // TFCAN_MOType
		uint32_t identifier:29;
	};
	uint8_t data[8];
	uint8_t length;
} __attribute__((__packed__)) TFCAN_Frame; // 13 bytes

typedef struct {
	bool reconfigure_transceiver;
	uint32_t baud_rate; // [10000..10000000] bps
	uint32_t baud_rate_new;
	uint16_t sample_point; // [0..10000] 0.01 %
	uint8_t sync_jump_width; // [1..4] // FIXME: bit-timing calculation assumes this to be 1
	TFCAN_TransceiverMode transceiver_mode;
	TFCAN_TransceiverMode transceiver_mode_new;

	CAN_NODE_TypeDef *node[TFCAN_NODE_SIZE];
	CAN_NODE_TypeDef *tx_node;
	CAN_NODE_TypeDef *rx_node;

	CAN_MO_TypeDef *mo[TFCAN_MO_SIZE];
	int32_t mo_frame_counter[TFCAN_MO_SIZE];
	uint16_t mo_age[TFCAN_MO_SIZE];

	uint8_t tx_buffer_size; // [0..TFCAN_BUFFER_SIZE]
	CAN_MO_TypeDef **tx_buffer_mo;
	uint8_t tx_buffer_mo_next_index;
	uint32_t tx_buffer_mo_timestamp[TFCAN_MO_SIZE];
	int32_t tx_buffer_timeout; // < 0 (single-shot), = 0 (infinite), > 0 (milliseconds)
	bool tx_buffer_timeout_pending;
	uint8_t tx_buffer_timeout_mo_index;
	uint32_t tx_buffer_timeout_settle_timestamp;

	uint8_t rx_buffer_size[TFCAN_RX_BUFFER_SIZE]; // [0..TFCAN_RX_BUFFER_SIZE]
	TFCAN_BufferType rx_buffer_type[TFCAN_RX_BUFFER_SIZE];
	CAN_MO_TypeDef **rx_buffer_mo[TFCAN_RX_BUFFER_SIZE];
	uint8_t rx_buffer_mo_next_index[TFCAN_RX_BUFFER_SIZE];
	int32_t *rx_buffer_mo_frame_counter[TFCAN_RX_BUFFER_SIZE];
	uint16_t *rx_buffer_mo_age[TFCAN_RX_BUFFER_SIZE];

	TFCAN_Frame backlog[TFCAN_BACKLOG_SIZE];

	uint16_t tx_backlog_size; // [0..TFCAN_BACKLOG_SIZE]
	TFCAN_Frame *tx_backlog;
	uint16_t tx_backlog_start;
	uint16_t tx_backlog_end;

	uint16_t rx_backlog_size; // [0..TFCAN_BACKLOG_SIZE]
	TFCAN_Frame *rx_backlog;
	uint16_t rx_backlog_start;
	uint16_t rx_backlog_end;

	uint32_t last_debug;
} TFCAN; // avoid name collision with global XMC CAN object named CAN

void tfcan_init(void);
void tfcan_tick(void);

void tfcan_set_config_mode(const bool enable);

void tfcan_reconfigure_transceiver(void);
void tfcan_reconfigure_queues(void);

void tfcan_check_tx_buffer_timeout(void);

bool tfcan_enqueue_frame(TFCAN_Frame *frame);
bool tfcan_dequeue_frame(TFCAN_Frame *frame);

#endif
