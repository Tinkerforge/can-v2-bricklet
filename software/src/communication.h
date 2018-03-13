/* can-v2-bricklet
 * Copyright (C) 2018 Matthias Bolte <matthias@tinkerforge.com>
 *
 * communication.h: TFP protocol message handling
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

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>
#include <stdbool.h>

#include "bricklib2/protocols/tfp/tfp.h"
#include "bricklib2/bootloader/bootloader.h"

// Default functions
BootloaderHandleMessageResponse handle_message(const void *data, void *response);
void communication_tick(void);
void communication_init(void);

// Constants
#define CAN_V2_FRAME_TYPE_STANDARD_DATA 0
#define CAN_V2_FRAME_TYPE_STANDARD_REMOTE 1
#define CAN_V2_FRAME_TYPE_EXTENDED_DATA 2
#define CAN_V2_FRAME_TYPE_EXTENDED_REMOTE 3

#define CAN_V2_TRANSCEIVER_MODE_NORMAL 0
#define CAN_V2_TRANSCEIVER_MODE_LOOPBACK 1
#define CAN_V2_TRANSCEIVER_MODE_READ_ONLY 2

#define CAN_V2_BUFFER_TYPE_DATA 0
#define CAN_V2_BUFFER_TYPE_REMOTE 1

#define CAN_V2_FILTER_MODE_ACCEPT_ALL 0
#define CAN_V2_FILTER_MODE_MATCH_STANDARD_AND_EXTENDED 1
#define CAN_V2_FILTER_MODE_MATCH_STANDARD_ONLY 2
#define CAN_V2_FILTER_MODE_MATCH_EXTENDED_ONLY 3

#define CAN_V2_BOOTLOADER_MODE_BOOTLOADER 0
#define CAN_V2_BOOTLOADER_MODE_FIRMWARE 1
#define CAN_V2_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2
#define CAN_V2_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3
#define CAN_V2_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

#define CAN_V2_BOOTLOADER_STATUS_OK 0
#define CAN_V2_BOOTLOADER_STATUS_INVALID_MODE 1
#define CAN_V2_BOOTLOADER_STATUS_NO_CHANGE 2
#define CAN_V2_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3
#define CAN_V2_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4
#define CAN_V2_BOOTLOADER_STATUS_CRC_MISMATCH 5

#define CAN_V2_STATUS_LED_CONFIG_OFF 0
#define CAN_V2_STATUS_LED_CONFIG_ON 1
#define CAN_V2_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2
#define CAN_V2_STATUS_LED_CONFIG_SHOW_STATUS 3

// Function and callback IDs and structs
#define FID_WRITE_FRAME_LOW_LEVEL 1
#define FID_READ_FRAME_LOW_LEVEL 2
#define FID_SET_FRAME_READ_CALLBACK_CONFIGURATION 3
#define FID_GET_FRAME_READ_CALLBACK_CONFIGURATION 4
#define FID_SET_TRANSCEIVER_CONFIGURATION 5
#define FID_GET_TRANSCEIVER_CONFIGURATION 6
#define FID_SET_WRITE_QUEUE_CONFIGURATION 7
#define FID_GET_WRITE_QUEUE_CONFIGURATION 8
#define FID_SET_READ_QUEUE_BUFFER_CONFIGURATION 9
#define FID_GET_READ_QUEUE_BUFFER_CONFIGURATION 10
#define FID_SET_READ_QUEUE_BACKLOG_CONFIGURATION 11
#define FID_GET_READ_QUEUE_BACKLOG_CONFIGURATION 12
#define FID_GET_ERROR_LOG 13

#define FID_CALLBACK_FRAME_READ_LOW_LEVEL 14

typedef struct {
	TFPMessageHeader header;
	uint8_t frame_type;
	uint32_t identifier;
	uint8_t data_length;
	uint8_t data_data[15];
} __attribute__((__packed__)) WriteFrameLowLevel;

typedef struct {
	TFPMessageHeader header;
	bool success;
} __attribute__((__packed__)) WriteFrameLowLevel_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) ReadFrameLowLevel;

typedef struct {
	TFPMessageHeader header;
	bool success;
	uint8_t frame_type;
	uint32_t identifier;
	uint8_t data_length;
	uint8_t data_data[15];
} __attribute__((__packed__)) ReadFrameLowLevel_Response;

typedef struct {
	TFPMessageHeader header;
	bool enabled;
} __attribute__((__packed__)) SetFrameReadCallbackConfiguration;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetFrameReadCallbackConfiguration;

typedef struct {
	TFPMessageHeader header;
	bool enabled;
} __attribute__((__packed__)) GetFrameReadCallbackConfiguration_Response;

typedef struct {
	TFPMessageHeader header;
	uint32_t baud_rate;
	uint8_t transceiver_mode;
} __attribute__((__packed__)) SetTransceiverConfiguration;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetTransceiverConfiguration;

typedef struct {
	TFPMessageHeader header;
	uint32_t baud_rate;
	uint8_t transceiver_mode;
} __attribute__((__packed__)) GetTransceiverConfiguration_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t buffer_size;
	int32_t buffer_timeout;
	uint16_t backlog_size;
} __attribute__((__packed__)) SetWriteQueueConfiguration;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetWriteQueueConfiguration;

typedef struct {
	TFPMessageHeader header;
	uint8_t buffer_size;
	int32_t buffer_timeout;
	uint16_t backlog_size;
} __attribute__((__packed__)) GetWriteQueueConfiguration_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t buffer_index;
	uint8_t buffer_type;
	uint8_t buffer_size;
	uint8_t filter_mode;
	uint32_t filter_mask;
	uint32_t filter_match;
} __attribute__((__packed__)) SetReadQueueBufferConfiguration;

typedef struct {
	TFPMessageHeader header;
	uint8_t buffer_index;
} __attribute__((__packed__)) GetReadQueueBufferConfiguration;

typedef struct {
	TFPMessageHeader header;
	uint8_t buffer_type;
	uint8_t buffer_size;
	uint8_t filter_mode;
	uint32_t filter_mask;
	uint32_t filter_match;
} __attribute__((__packed__)) GetReadQueueBufferConfiguration_Response;

typedef struct {
	TFPMessageHeader header;
	uint16_t backlog_size;
} __attribute__((__packed__)) SetReadQueueBacklogConfiguration;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetReadQueueBacklogConfiguration;

typedef struct {
	TFPMessageHeader header;
	uint16_t backlog_size;
} __attribute__((__packed__)) GetReadQueueBacklogConfiguration_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetErrorLog;

typedef struct {
	TFPMessageHeader header;
	uint8_t write_error_level;
	uint8_t read_error_level;
	bool transceiver_disabled;
	uint32_t write_timeout_count;
	uint32_t read_register_overflow_count;
	uint32_t read_buffer_overflow_count;
} __attribute__((__packed__)) GetErrorLog_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t frame_type;
	uint32_t identifier;
	uint8_t data_length;
	uint8_t data_data[15];
} __attribute__((__packed__)) FrameReadLowLevel_Callback;


// Function prototypes
BootloaderHandleMessageResponse write_frame_low_level(const WriteFrameLowLevel *data, WriteFrameLowLevel_Response *response);
BootloaderHandleMessageResponse read_frame_low_level(const ReadFrameLowLevel *data, ReadFrameLowLevel_Response *response);
BootloaderHandleMessageResponse set_frame_read_callback_configuration(const SetFrameReadCallbackConfiguration *data);
BootloaderHandleMessageResponse get_frame_read_callback_configuration(const GetFrameReadCallbackConfiguration *data, GetFrameReadCallbackConfiguration_Response *response);
BootloaderHandleMessageResponse set_transceiver_configuration(const SetTransceiverConfiguration *data);
BootloaderHandleMessageResponse get_transceiver_configuration(const GetTransceiverConfiguration *data, GetTransceiverConfiguration_Response *response);
BootloaderHandleMessageResponse set_write_queue_configuration(const SetWriteQueueConfiguration *data);
BootloaderHandleMessageResponse get_write_queue_configuration(const GetWriteQueueConfiguration *data, GetWriteQueueConfiguration_Response *response);
BootloaderHandleMessageResponse set_read_queue_buffer_configuration(const SetReadQueueBufferConfiguration *data);
BootloaderHandleMessageResponse get_read_queue_buffer_configuration(const GetReadQueueBufferConfiguration *data, GetReadQueueBufferConfiguration_Response *response);
BootloaderHandleMessageResponse set_read_queue_backlog_configuration(const SetReadQueueBacklogConfiguration *data);
BootloaderHandleMessageResponse get_read_queue_backlog_configuration(const GetReadQueueBacklogConfiguration *data, GetReadQueueBacklogConfiguration_Response *response);
BootloaderHandleMessageResponse get_error_log(const GetErrorLog *data, GetErrorLog_Response *response);

// Callbacks
bool handle_frame_read_low_level_callback(void);

#define COMMUNICATION_CALLBACK_TICK_WAIT_MS 1
#define COMMUNICATION_CALLBACK_HANDLER_NUM 1
#define COMMUNICATION_CALLBACK_LIST_INIT \
	handle_frame_read_low_level_callback, \


#endif
