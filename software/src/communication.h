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

#define CAN_V2_FILTER_MODE_ACCEPT_ALL 0
#define CAN_V2_FILTER_MODE_MATCH_STANDARD_ONLY 1
#define CAN_V2_FILTER_MODE_MATCH_EXTENDED_ONLY 2
#define CAN_V2_FILTER_MODE_MATCH_STANDARD_AND_EXTENDED 3

#define CAN_V2_TRANSCEIVER_STATE_ACTIVE 0
#define CAN_V2_TRANSCEIVER_STATE_PASSIVE 1
#define CAN_V2_TRANSCEIVER_STATE_DISABLED 2

#define CAN_V2_COMMUNICATION_LED_CONFIG_OFF 0
#define CAN_V2_COMMUNICATION_LED_CONFIG_ON 1
#define CAN_V2_COMMUNICATION_LED_CONFIG_SHOW_HEARTBEAT 2
#define CAN_V2_COMMUNICATION_LED_CONFIG_SHOW_COMMUNICATION 3

#define CAN_V2_ERROR_LED_CONFIG_OFF 0
#define CAN_V2_ERROR_LED_CONFIG_ON 1
#define CAN_V2_ERROR_LED_CONFIG_SHOW_HEARTBEAT 2
#define CAN_V2_ERROR_LED_CONFIG_SHOW_TRANSCEIVER_STATE 3
#define CAN_V2_ERROR_LED_CONFIG_SHOW_ERROR 4

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
#define FID_SET_QUEUE_CONFIGURATION_LOW_LEVEL 7
#define FID_GET_QUEUE_CONFIGURATION_LOW_LEVEL 8
#define FID_SET_READ_FILTER_CONFIGURATION 9
#define FID_GET_READ_FILTER_CONFIGURATION 10
#define FID_GET_ERROR_LOG_LOW_LEVEL 11
#define FID_SET_COMMUNICATION_LED_CONFIG 12
#define FID_GET_COMMUNICATION_LED_CONFIG 13
#define FID_SET_ERROR_LED_CONFIG 14
#define FID_GET_ERROR_LED_CONFIG 15

#define FID_CALLBACK_FRAME_READ_LOW_LEVEL 16

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
	uint16_t sample_point;
	uint8_t transceiver_mode;
} __attribute__((__packed__)) SetTransceiverConfiguration;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetTransceiverConfiguration;

typedef struct {
	TFPMessageHeader header;
	uint32_t baud_rate;
	uint16_t sample_point;
	uint8_t transceiver_mode;
} __attribute__((__packed__)) GetTransceiverConfiguration_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t write_buffer_size;
	int32_t write_buffer_timeout;
	uint16_t write_backlog_size;
	uint8_t read_buffer_sizes_length;
	int8_t read_buffer_sizes_data[32];
	uint16_t read_backlog_size;
} __attribute__((__packed__)) SetQueueConfigurationLowLevel;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetQueueConfigurationLowLevel;

typedef struct {
	TFPMessageHeader header;
	uint8_t write_buffer_size;
	int32_t write_buffer_timeout;
	uint16_t write_backlog_size;
	uint8_t read_buffer_sizes_length;
	int8_t read_buffer_sizes_data[32];
	uint16_t read_backlog_size;
} __attribute__((__packed__)) GetQueueConfigurationLowLevel_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t buffer_index;
	uint8_t filter_mode;
	uint32_t filter_mask;
	uint32_t filter_identifier;
} __attribute__((__packed__)) SetReadFilterConfiguration;

typedef struct {
	TFPMessageHeader header;
	uint8_t buffer_index;
} __attribute__((__packed__)) GetReadFilterConfiguration;

typedef struct {
	TFPMessageHeader header;
	uint8_t filter_mode;
	uint32_t filter_mask;
	uint32_t filter_identifier;
} __attribute__((__packed__)) GetReadFilterConfiguration_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetErrorLogLowLevel;

typedef struct {
	TFPMessageHeader header;
	uint8_t transceiver_state;
	uint8_t transceiver_write_error_level;
	uint8_t transceiver_read_error_level;
	uint32_t transceiver_stuffing_error_count;
	uint32_t transceiver_format_error_count;
	uint32_t transceiver_ack_error_count;
	uint32_t transceiver_bit1_error_count;
	uint32_t transceiver_bit0_error_count;
	uint32_t transceiver_crc_error_count;
	uint32_t write_buffer_timeout_error_count;
	uint32_t read_buffer_overflow_error_count;
	uint8_t read_buffer_overflow_error_occurred_length;
	uint32_t read_buffer_overflow_error_occurred_data;
	uint32_t read_backlog_overflow_error_count;
} __attribute__((__packed__)) GetErrorLogLowLevel_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t config;
} __attribute__((__packed__)) SetCommunicationLEDConfig;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetCommunicationLEDConfig;

typedef struct {
	TFPMessageHeader header;
	uint8_t config;
} __attribute__((__packed__)) GetCommunicationLEDConfig_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t config;
} __attribute__((__packed__)) SetErrorLEDConfig;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetErrorLEDConfig;

typedef struct {
	TFPMessageHeader header;
	uint8_t config;
} __attribute__((__packed__)) GetErrorLEDConfig_Response;

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
BootloaderHandleMessageResponse set_queue_configuration_low_level(const SetQueueConfigurationLowLevel *data);
BootloaderHandleMessageResponse get_queue_configuration_low_level(const GetQueueConfigurationLowLevel *data, GetQueueConfigurationLowLevel_Response *response);
BootloaderHandleMessageResponse set_read_filter_configuration(const SetReadFilterConfiguration *data);
BootloaderHandleMessageResponse get_read_filter_configuration(const GetReadFilterConfiguration *data, GetReadFilterConfiguration_Response *response);
BootloaderHandleMessageResponse get_error_log_low_level(const GetErrorLogLowLevel *data, GetErrorLogLowLevel_Response *response);
BootloaderHandleMessageResponse set_communication_led_config(const SetCommunicationLEDConfig *data);
BootloaderHandleMessageResponse get_communication_led_config(const GetCommunicationLEDConfig *data, GetCommunicationLEDConfig_Response *response);
BootloaderHandleMessageResponse set_error_led_config(const SetErrorLEDConfig *data);
BootloaderHandleMessageResponse get_error_led_config(const GetErrorLEDConfig *data, GetErrorLEDConfig_Response *response);

// Callbacks
bool handle_frame_read_low_level_callback(void);

#define COMMUNICATION_CALLBACK_TICK_WAIT_MS 1
#define COMMUNICATION_CALLBACK_HANDLER_NUM 1
#define COMMUNICATION_CALLBACK_LIST_INIT \
	handle_frame_read_low_level_callback, \

#endif
