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

#define CAN_V2_BAUD_RATE_10KBPS 0
#define CAN_V2_BAUD_RATE_20KBPS 1
#define CAN_V2_BAUD_RATE_50KBPS 2
#define CAN_V2_BAUD_RATE_125KBPS 3
#define CAN_V2_BAUD_RATE_250KBPS 4
#define CAN_V2_BAUD_RATE_500KBPS 5
#define CAN_V2_BAUD_RATE_800KBPS 6
#define CAN_V2_BAUD_RATE_1000KBPS 7

#define CAN_V2_TRANSCEIVER_MODE_NORMAL 0
#define CAN_V2_TRANSCEIVER_MODE_LOOPBACK 1
#define CAN_V2_TRANSCEIVER_MODE_READ_ONLY 2

#define CAN_V2_FILTER_MODE_DISABLED 0
#define CAN_V2_FILTER_MODE_ACCEPT_ALL 1
#define CAN_V2_FILTER_MODE_MATCH_STANDARD 2
#define CAN_V2_FILTER_MODE_MATCH_STANDARD_AND_DATA 3
#define CAN_V2_FILTER_MODE_MATCH_EXTENDED 4

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
#define FID_WRITE_FRAME 1
#define FID_READ_FRAME 2
#define FID_ENABLE_FRAME_READ_CALLBACK 3
#define FID_DISABLE_FRAME_READ_CALLBACK 4
#define FID_IS_FRAME_READ_CALLBACK_ENABLED 5
#define FID_SET_CONFIGURATION 6
#define FID_GET_CONFIGURATION 7
#define FID_SET_READ_FILTER 8
#define FID_GET_READ_FILTER 9
#define FID_GET_ERROR_LOG 10

#define FID_CALLBACK_FRAME_READ 11

typedef struct {
	TFPMessageHeader header;
	uint8_t frame_type;
	uint32_t identifier;
	uint8_t data[8];
	uint8_t length;
} __attribute__((__packed__)) WriteFrame;

typedef struct {
	TFPMessageHeader header;
	bool success;
} __attribute__((__packed__)) WriteFrame_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) ReadFrame;

typedef struct {
	TFPMessageHeader header;
	bool success;
	uint8_t frame_type;
	uint32_t identifier;
	uint8_t data[8];
	uint8_t length;
} __attribute__((__packed__)) ReadFrame_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) EnableFrameReadCallback;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) DisableFrameReadCallback;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) IsFrameReadCallbackEnabled;

typedef struct {
	TFPMessageHeader header;
	bool enabled;
} __attribute__((__packed__)) IsFrameReadCallbackEnabled_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t baud_rate;
	uint8_t transceiver_mode;
	int32_t write_timeout;
} __attribute__((__packed__)) SetConfiguration;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetConfiguration;

typedef struct {
	TFPMessageHeader header;
	uint8_t baud_rate;
	uint8_t transceiver_mode;
	int32_t write_timeout;
} __attribute__((__packed__)) GetConfiguration_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t mode;
	uint32_t mask;
	uint32_t filter1;
	uint32_t filter2;
} __attribute__((__packed__)) SetReadFilter;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetReadFilter;

typedef struct {
	TFPMessageHeader header;
	uint8_t mode;
	uint32_t mask;
	uint32_t filter1;
	uint32_t filter2;
} __attribute__((__packed__)) GetReadFilter_Response;

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
	uint8_t data[8];
	uint8_t length;
} __attribute__((__packed__)) FrameRead_Callback;


// Function prototypes
BootloaderHandleMessageResponse write_frame(const WriteFrame *data, WriteFrame_Response *response);
BootloaderHandleMessageResponse read_frame(const ReadFrame *data, ReadFrame_Response *response);
BootloaderHandleMessageResponse enable_frame_read_callback(const EnableFrameReadCallback *data);
BootloaderHandleMessageResponse disable_frame_read_callback(const DisableFrameReadCallback *data);
BootloaderHandleMessageResponse is_frame_read_callback_enabled(const IsFrameReadCallbackEnabled *data, IsFrameReadCallbackEnabled_Response *response);
BootloaderHandleMessageResponse set_configuration(const SetConfiguration *data);
BootloaderHandleMessageResponse get_configuration(const GetConfiguration *data, GetConfiguration_Response *response);
BootloaderHandleMessageResponse set_read_filter(const SetReadFilter *data);
BootloaderHandleMessageResponse get_read_filter(const GetReadFilter *data, GetReadFilter_Response *response);
BootloaderHandleMessageResponse get_error_log(const GetErrorLog *data, GetErrorLog_Response *response);

// Callbacks
bool handle_frame_read_callback(void);

#define COMMUNICATION_CALLBACK_TICK_WAIT_MS 1
#define COMMUNICATION_CALLBACK_HANDLER_NUM 1
#define COMMUNICATION_CALLBACK_LIST_INIT \
	handle_frame_read_callback, \

#endif
