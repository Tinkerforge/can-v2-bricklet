/* can-v2-bricklet
 * Copyright (C) 2018-2019 Matthias Bolte <matthias@tinkerforge.com>
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

#include "bricklib2/utility/led_flicker.h"

#include "xmc_can.h"

#include "tfcan_mo.h"

// must match Bricklet API TransceiverMode enum
typedef enum {
	TFCAN_TRANSCEIVER_MODE_NORMAL = 0,
	TFCAN_TRANSCEIVER_MODE_LOOPBACK,
	TFCAN_TRANSCEIVER_MODE_READ_ONLY
} TFCAN_TransceiverMode;

// must match Bricklet API TransceiverState enum
typedef enum {
	TFCAN_TRANSCEIVER_STATE_ACTIVE = 0,
	TFCAN_TRANSCEIVER_STATE_PASSIVE,
	TFCAN_TRANSCEIVER_STATE_DISABLED
} TFCAN_TransceiverState;

// must match Bricklet API ErrorLEDConfig enum
typedef enum {
	TFCAN_ERROR_LED_CONFIG_OFF = 0,
	TFCAN_ERROR_LED_CONFIG_ON,
	TFCAN_ERROR_LED_CONFIG_SHOW_HEARTBEAT,
	TFCAN_ERROR_LED_CONFIG_SHOW_TRANSCEIVER_STATE,
	TFCAN_ERROR_LED_CONFIG_SHOW_ERROR
} TFCAN_ErrorLEDConfig;

typedef enum {
	TFCAN_NODE_LEC_STUFFING_ERROR = 0b001,
	TFCAN_NODE_LEC_FORMAT_ERROR   = 0b010,
	TFCAN_NODE_LEC_ACK_ERROR      = 0b011,
	TFCAN_NODE_LEC_BIT1_ERROR     = 0b100,
	TFCAN_NODE_LEC_BIT0_ERROR     = 0b101,
	TFCAN_NODE_LEC_CRC_ERROR      = 0b110
} TFCAN_NodeLEC;

typedef struct {
	struct {
		uint32_t mo_type:3; // TFCAN_MOType
		uint32_t identifier:29;
	};
	uint8_t data[8];
	uint8_t length;
} __attribute__((__packed__)) TFCAN_Frame; // 13 bytes

typedef struct {
	TFCAN_Frame frame;
	uint64_t timestamp; // microsecond
} __attribute__((__packed__)) TFCAN_TimestampedFrame; // 21 bytes

typedef struct {
	bool reconfigure_transceiver;
	bool reconfigure_queues;
	uint32_t reconfigure_rx_filters; // bitmask

	uint32_t baud_rate; // config, [10000..10000000] bit/s
	uint16_t sample_point; // config, [500..900] 0.1 %
	uint8_t sync_jump_width; // [1..4] // FIXME: bit-timing calculation assumes this to be 1
	TFCAN_TransceiverMode transceiver_mode; // config
	bool timestamped_frame_enabled; // config

	CAN_NODE_TypeDef *node[TFCAN_NODE_SIZE];
	CAN_NODE_TypeDef *tx_node;
	CAN_NODE_TypeDef *rx_node;

	CAN_MO_TypeDef *mo[TFCAN_BUFFER_SIZE];
	int32_t mo_frame_counter[TFCAN_BUFFER_SIZE];
	uint16_t mo_age[TFCAN_BUFFER_SIZE];

	uint8_t tx_buffer_size; // config, [0..TFCAN_BUFFER_SIZE]
	CAN_MO_TypeDef **tx_buffer_mo;
	uint8_t tx_buffer_mo_next_index;
	uint32_t tx_buffer_mo_timestamp[TFCAN_BUFFER_SIZE];
	int32_t tx_buffer_timeout; // config, < 0 (single-shot), = 0 (infinite), > 0 (milliseconds)
	bool tx_buffer_timeout_pending;
	uint8_t tx_buffer_timeout_mo_index;
	uint32_t tx_buffer_timeout_settle_timestamp;

	uint8_t rx_buffer_size[TFCAN_BUFFER_SIZE]; // config, [0..TFCAN_BUFFER_SIZE]
	TFCAN_BufferType rx_buffer_type[TFCAN_BUFFER_SIZE]; // config
	CAN_MO_TypeDef **rx_buffer_mo[TFCAN_BUFFER_SIZE];
	uint8_t rx_buffer_mo_next_index[TFCAN_BUFFER_SIZE];
	int32_t *rx_buffer_mo_frame_counter[TFCAN_BUFFER_SIZE];
	uint16_t *rx_buffer_mo_age[TFCAN_BUFFER_SIZE];

	union {
		TFCAN_Frame backlog[TFCAN_BACKLOG_SIZE];
		TFCAN_TimestampedFrame timestamped_backlog[TFCAN_TIMESTAMPED_BACKLOG_SIZE];
	};

	uint16_t tx_backlog_size; // config, [0..TFCAN_BACKLOG_SIZE]
	uint16_t tx_timestamped_backlog_size; // config, [0..TFCAN_TIMESTAMPED_BACKLOG_SIZE]
	TFCAN_Frame *tx_backlog;
	TFCAN_TimestampedFrame *tx_timestamped_backlog;
	uint16_t tx_backlog_start;
	uint16_t tx_backlog_end;

	uint16_t rx_backlog_size; // config, [0..TFCAN_BACKLOG_SIZE]
	uint16_t rx_timestamped_backlog_size; // config, [0..TFCAN_TIMESTAMPED_BACKLOG_SIZE]
	TFCAN_Frame *rx_backlog;
	TFCAN_TimestampedFrame *rx_timestamped_backlog;
	uint16_t rx_backlog_start;
	uint16_t rx_backlog_end;

	TFCAN_FilterMode rx_filter_mode[TFCAN_BUFFER_SIZE]; // config
	uint32_t rx_filter_mask[TFCAN_BUFFER_SIZE]; // config
	uint32_t rx_filter_identifier[TFCAN_BUFFER_SIZE]; // config

	TFCAN_TransceiverState transceiver_state;
	uint8_t transceiver_tx_error_level;
	uint8_t transceiver_rx_error_level;
	uint32_t transceiver_stuffing_error_count;
	uint32_t transceiver_format_error_count;
	uint32_t transceiver_ack_error_count;
	uint32_t transceiver_bit1_error_count;
	uint32_t transceiver_bit0_error_count;
	uint32_t transceiver_crc_error_count;
	uint32_t tx_buffer_timeout_error_count;
	uint32_t rx_buffer_overflow_error_count;
	uint32_t rx_buffer_overflow_error_occurred; // bitmask
	uint32_t rx_backlog_overflow_error_count;

	LEDFlickerState com_led_state;
	LEDFlickerState error_led_state;
	TFCAN_ErrorLEDConfig error_led_config;

#ifdef TFCAN_BUFFER_DEBUG
	uint32_t last_buffer_debug;
#endif
} TFCAN; // avoid name collision with global XMC CAN object named CAN

void tfcan_init(void);
void tfcan_tick(void);

void tfcan_set_config_mode(const bool enable);

void tfcan_reconfigure_transceiver(void);
void tfcan_reconfigure_queues(void);
void tfcan_reconfigure_rx_filters(void);

void tfcan_check_tx_buffer_timeout(void);

bool tfcan_frame_readable();

bool tfcan_enqueue_frame(TFCAN_Frame *frame);
bool tfcan_enqueue_timestamped_frame(TFCAN_TimestampedFrame *frame);
bool tfcan_dequeue_frame(TFCAN_Frame *frame);
bool tfcan_dequeue_timestamped_frame(TFCAN_TimestampedFrame *frame);

#endif
