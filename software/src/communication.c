/* can-v2-bricklet
 * Copyright (C) 2018-2019 Matthias Bolte <matthias@tinkerforge.com>
 *
 * communication.c: TFP protocol message handling
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

#include "communication.h"

#include "bricklib2/utility/communication_callback.h"
#include "bricklib2/utility/util_definitions.h"
#include "bricklib2/protocols/tfp/tfp.h"
#include "bricklib2/logging/logging.h"

#include "tfcan.h"

extern TFCAN tfcan;

static bool frame_read_callback_enabled = false;
static bool frame_readable_callback_enabled = false;
static bool frame_readable_callback_already_sent = false;
static bool error_occurred_callback_enabled = false;
static bool timestamped_frame_read_callback_enabled = false;

BootloaderHandleMessageResponse handle_message(const void *message, void *response) {
	switch(tfp_get_fid_from_message(message)) {
		case FID_WRITE_FRAME_LOW_LEVEL: return write_frame_low_level(message, response);
		case FID_READ_FRAME_LOW_LEVEL: return read_frame_low_level(message, response);
		case FID_SET_FRAME_READ_CALLBACK_CONFIGURATION: return set_frame_read_callback_configuration(message);
		case FID_GET_FRAME_READ_CALLBACK_CONFIGURATION: return get_frame_read_callback_configuration(message, response);
		case FID_SET_TRANSCEIVER_CONFIGURATION: return set_transceiver_configuration(message);
		case FID_GET_TRANSCEIVER_CONFIGURATION: return get_transceiver_configuration(message, response);
		case FID_SET_QUEUE_CONFIGURATION_LOW_LEVEL: return set_queue_configuration_low_level(message);
		case FID_GET_QUEUE_CONFIGURATION_LOW_LEVEL: return get_queue_configuration_low_level(message, response);
		case FID_SET_READ_FILTER_CONFIGURATION: return set_read_filter_configuration(message);
		case FID_GET_READ_FILTER_CONFIGURATION: return get_read_filter_configuration(message, response);
		case FID_GET_ERROR_LOG_LOW_LEVEL: return get_error_log_low_level(message, response);
		case FID_SET_COMMUNICATION_LED_CONFIG: return set_communication_led_config(message);
		case FID_GET_COMMUNICATION_LED_CONFIG: return get_communication_led_config(message, response);
		case FID_SET_ERROR_LED_CONFIG: return set_error_led_config(message);
		case FID_GET_ERROR_LED_CONFIG: return get_error_led_config(message, response);
		case FID_SET_FRAME_READABLE_CALLBACK_CONFIGURATION: return set_frame_readable_callback_configuration(message);
		case FID_GET_FRAME_READABLE_CALLBACK_CONFIGURATION: return get_frame_readable_callback_configuration(message, response);
		case FID_SET_ERROR_OCCURRED_CALLBACK_CONFIGURATION: return set_error_occurred_callback_configuration(message);
		case FID_GET_ERROR_OCCURRED_CALLBACK_CONFIGURATION: return get_error_occurred_callback_configuration(message, response);
/*
		case FID_SET_TIMESTAMPED_FRAME_CONFIGURATION: return set_timestamped_frame_configuration(message);
		case FID_GET_TIMESTAMPED_FRAME_CONFIGURATION: return get_timestamped_frame_configuration(message, response);
		case FID_WRITE_TIMESTAMPED_FRAME_LOW_LEVEL: return write_timestamped_frame_low_level(message, response);
		case FID_READ_TIMESTAMPED_FRAME_LOW_LEVEL: return read_timestamped_frame_low_level(message, response);
		case FID_GET_TIMESTAMP: return get_timestamp(message, response);
		case FID_SET_TIMESTAMPED_FRAME_READ_CALLBACK_CONFIGURATION: return set_timestamped_frame_read_callback_configuration(message);
		case FID_GET_TIMESTAMPED_FRAME_READ_CALLBACK_CONFIGURATION: return get_timestamped_frame_read_callback_configuration(message, response);
*/
		default: return HANDLE_MESSAGE_RESPONSE_NOT_SUPPORTED;
	}
}

BootloaderHandleMessageResponse write_frame_low_level(const WriteFrameLowLevel *data, WriteFrameLowLevel_Response *response) {
	if (data->frame_type > CAN_V2_FRAME_TYPE_EXTENDED_REMOTE ||
	    data->data_length > 15) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	if (data->frame_type < CAN_V2_FRAME_TYPE_EXTENDED_DATA) {
		if (data->identifier >= (1u << 11)) {
			return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
		}
	} else {
		if (data->identifier >= (1u << 29)) {
			return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
		}
	}

	TFCAN_Frame frame;

	frame.mo_type    = data->frame_type;
	frame.identifier = data->identifier;
	memcpy(frame.data, data->data_data, MIN(data->data_length, sizeof(frame.data)));
	frame.length     = data->data_length;

	response->header.length = sizeof(WriteFrameLowLevel_Response);
	response->success       = tfcan_enqueue_frame(&frame);

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse read_frame_low_level(const ReadFrameLowLevel *data, ReadFrameLowLevel_Response *response) {
	response->header.length = sizeof(ReadFrameLowLevel_Response);
	frame_readable_callback_already_sent = false;

	if (frame_read_callback_enabled || timestamped_frame_read_callback_enabled) {
		response->success     = false;
		response->frame_type  = CAN_V2_FRAME_TYPE_STANDARD_DATA;
		response->identifier  = 0;
		response->data_length = 0;
		memset(&response->data_data, 0, sizeof(response->data_data));
	} else {
		// Need to zero the whole frame here because tfcan_dequeue_frame
		// will not touch it if there is no frame to be read
		TFCAN_Frame frame = {{0}};

		response->success     = tfcan_dequeue_frame(&frame);

		const uint8_t length = MIN(frame.length, 8);

		response->frame_type  = frame.mo_type;
		response->identifier  = frame.identifier;
		response->data_length = frame.length;
		memcpy(response->data_data, frame.data, length);
		memset(&response->data_data[length], 0, sizeof(response->data_data) - length);
	}

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_frame_read_callback_configuration(const SetFrameReadCallbackConfiguration *data) {
	frame_read_callback_enabled = data->enabled;
	if (data->enabled) {
		frame_readable_callback_enabled = false;
	}

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_frame_read_callback_configuration(const GetFrameReadCallbackConfiguration *data, GetFrameReadCallbackConfiguration_Response *response) {
	response->header.length = sizeof(GetFrameReadCallbackConfiguration_Response);
	response->enabled       = frame_read_callback_enabled;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_transceiver_configuration(const SetTransceiverConfiguration *data) {
	if (data->baud_rate < 10000 || data->baud_rate > 1000000 ||
	    data->sample_point < 500 || data->sample_point > 900 ||
	    data->transceiver_mode > CAN_V2_TRANSCEIVER_MODE_READ_ONLY) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	tfcan.reconfigure_transceiver = true;
	tfcan.baud_rate               = data->baud_rate;
	tfcan.sample_point            = data->sample_point;
	tfcan.transceiver_mode        = data->transceiver_mode;

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_transceiver_configuration(const GetTransceiverConfiguration *data, GetTransceiverConfiguration_Response *response) {
	response->header.length    = sizeof(GetTransceiverConfiguration_Response);
	response->baud_rate        = tfcan.baud_rate;
	response->sample_point     = tfcan.sample_point;
	response->transceiver_mode = tfcan.transceiver_mode;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_queue_configuration_low_level(const SetQueueConfigurationLowLevel *data) {
	if (data->read_buffer_sizes_length > TFCAN_BUFFER_SIZE ||
	    data->write_backlog_size + data->read_backlog_size > TFCAN_BACKLOG_SIZE) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	uint16_t total_buffer_size = data->write_buffer_size;

	for (uint8_t i = 0; i < data->read_buffer_sizes_length; ++i) {
		if (data->read_buffer_sizes_data[i] == 0) {
			return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
		}

		total_buffer_size += ABS(data->read_buffer_sizes_data[i]);
	}

	if (total_buffer_size > TFCAN_BUFFER_SIZE) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	tfcan.reconfigure_queues    = true;
	tfcan.tx_buffer_size        = data->write_buffer_size;
	tfcan.tx_buffer_timeout     = MAX(data->write_buffer_timeout, -1);
	tfcan.tx_backlog_size       = data->write_backlog_size;

	for (uint8_t i = 0; i < data->read_buffer_sizes_length; ++i) {
		tfcan.rx_buffer_size[i] = ABS(data->read_buffer_sizes_data[i]);
		tfcan.rx_buffer_type[i] = data->read_buffer_sizes_data[i] >= 0 ? TFCAN_BUFFER_TYPE_DATA : TFCAN_BUFFER_TYPE_REMOTE;
	}

	for (uint8_t i = data->read_buffer_sizes_length; i < TFCAN_BUFFER_SIZE; ++i) {
		tfcan.rx_buffer_size[i] = 0;
		tfcan.rx_buffer_type[i] = TFCAN_BUFFER_TYPE_DATA;
	}

	tfcan.rx_backlog_size       = data->read_backlog_size;

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_queue_configuration_low_level(const GetQueueConfigurationLowLevel *data, GetQueueConfigurationLowLevel_Response *response) {
	response->header.length        = sizeof(GetQueueConfigurationLowLevel_Response);
	response->write_buffer_size    = tfcan.tx_buffer_size;
	response->write_buffer_timeout = tfcan.tx_buffer_timeout;
	response->write_backlog_size   = tfcan.tx_backlog_size;

	for (uint8_t i = 0; i < TFCAN_BUFFER_SIZE; ++i) {
		response->read_buffer_sizes_data[i] = (int8_t)tfcan.rx_buffer_size[i] * (tfcan.rx_buffer_type[i] == TFCAN_BUFFER_TYPE_DATA ? 1 : -1);

		if (tfcan.rx_buffer_size[i] != 0) {
			response->read_buffer_sizes_length = i + 1;
		}
	}

	response->read_backlog_size    = tfcan.rx_backlog_size;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_read_filter_configuration(const SetReadFilterConfiguration *data) {
	if (data->buffer_index >= TFCAN_BUFFER_SIZE ||
	    data->filter_mode > CAN_V2_FILTER_MODE_MATCH_STANDARD_AND_EXTENDED) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	if (data->filter_mode == CAN_V2_FILTER_MODE_MATCH_STANDARD_ONLY) {
		if (data->filter_mask >= (1u << 11) || data->filter_identifier >= (1u << 11)) {
			return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
		}
	} else if (data->filter_mode >= CAN_V2_FILTER_MODE_MATCH_EXTENDED_ONLY) {
		if (data->filter_mask >= (1u << 29) || data->filter_identifier >= (1u << 29)) {
			return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
		}
	}

	tfcan.reconfigure_rx_filters                      |= 1u << data->buffer_index;
	tfcan.rx_filter_mode[data->buffer_index]           = data->filter_mode;

	if (data->filter_mode != CAN_V2_FILTER_MODE_ACCEPT_ALL) {
		tfcan.rx_filter_mask[data->buffer_index]       = data->filter_mask;
		tfcan.rx_filter_identifier[data->buffer_index] = data->filter_identifier;
	} else {
		tfcan.rx_filter_mask[data->buffer_index]       = 0;
		tfcan.rx_filter_identifier[data->buffer_index] = 0;
	}

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_read_filter_configuration(const GetReadFilterConfiguration *data, GetReadFilterConfiguration_Response *response) {
	if (data->buffer_index >= TFCAN_BUFFER_SIZE) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	response->header.length     = sizeof(GetReadFilterConfiguration_Response);
	response->filter_mode       = tfcan.rx_filter_mode[data->buffer_index];
	response->filter_mask       = tfcan.rx_filter_mask[data->buffer_index];
	response->filter_identifier = tfcan.rx_filter_identifier[data->buffer_index];

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse get_error_log_low_level(const GetErrorLogLowLevel *data, GetErrorLogLowLevel_Response *response) {
	response->header.length                              = sizeof(GetErrorLogLowLevel_Response);
	response->transceiver_state                          = tfcan.transceiver_state;
	response->transceiver_write_error_level              = tfcan.transceiver_tx_error_level;
	response->transceiver_read_error_level               = tfcan.transceiver_rx_error_level;
	response->transceiver_stuffing_error_count           = tfcan.transceiver_stuffing_error_count;
	response->transceiver_format_error_count             = tfcan.transceiver_format_error_count;
	response->transceiver_ack_error_count                = tfcan.transceiver_ack_error_count;
	response->transceiver_bit1_error_count               = tfcan.transceiver_bit1_error_count;
	response->transceiver_bit0_error_count               = tfcan.transceiver_bit0_error_count;
	response->transceiver_crc_error_count                = tfcan.transceiver_crc_error_count;
	response->write_buffer_timeout_error_count           = tfcan.tx_buffer_timeout_error_count;
	response->read_buffer_overflow_error_count           = tfcan.rx_buffer_overflow_error_count;
	response->read_buffer_overflow_error_occurred_length = 0;

	for (uint8_t i = 0; i < TFCAN_BUFFER_SIZE; ++i) {
		if (tfcan.rx_buffer_size[i] != 0) {
			response->read_buffer_overflow_error_occurred_length = i + 1;
		}
	}

	response->read_buffer_overflow_error_occurred_data   = tfcan.rx_buffer_overflow_error_occurred & ((1u << response->read_buffer_overflow_error_occurred_length) - 1);
	response->read_backlog_overflow_error_count          = tfcan.rx_backlog_overflow_error_count;

	tfcan.rx_buffer_overflow_error_occurred = 0; // reading the error-log clears the occurred bitmask
	if (tfcan.error_state == TFCAN_ERROR_STATE_ERROR_REPORTED) {
		tfcan.error_state = TFCAN_ERROR_STATE_IDLE;
	}

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_communication_led_config(const SetCommunicationLEDConfig *data) {
	if (data->config > CAN_V2_COMMUNICATION_LED_CONFIG_SHOW_COMMUNICATION) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	tfcan.com_led_state.config = data->config;

	if (tfcan.com_led_state.config == LED_FLICKER_CONFIG_OFF) {
		XMC_GPIO_SetOutputHigh(TFCAN_COM_LED_PIN);
	} else {
		XMC_GPIO_SetOutputLow(TFCAN_COM_LED_PIN);
	}

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_communication_led_config(const GetCommunicationLEDConfig *data, GetCommunicationLEDConfig_Response *response) {
	response->header.length = sizeof(GetCommunicationLEDConfig_Response);
	response->config        = tfcan.com_led_state.config;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_error_led_config(const SetErrorLEDConfig *data) {
	if (data->config > CAN_V2_ERROR_LED_CONFIG_SHOW_ERROR) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	if (data->config >= CAN_V2_ERROR_LED_CONFIG_SHOW_TRANSCEIVER_STATE) {
		tfcan.error_led_state.config = LED_FLICKER_CONFIG_EXTERNAL;
	} else {
		tfcan.error_led_state.config = data->config;
	}

	tfcan.error_led_config = data->config;

	if (tfcan.error_led_state.config == LED_FLICKER_CONFIG_OFF ||
	    tfcan.error_led_state.config == LED_FLICKER_CONFIG_EXTERNAL) {
		XMC_GPIO_SetOutputHigh(TFCAN_ERROR_LED_PIN);
	} else {
		XMC_GPIO_SetOutputLow(TFCAN_ERROR_LED_PIN);
	}

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_error_led_config(const GetErrorLEDConfig *data, GetErrorLEDConfig_Response *response) {
	response->header.length = sizeof(GetErrorLEDConfig_Response);
	response->config        = tfcan.error_led_config;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_frame_readable_callback_configuration(const SetFrameReadableCallbackConfiguration *data) {
	frame_readable_callback_enabled = data->enabled;
	if(data->enabled) {
		frame_read_callback_enabled = false;
	}

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_frame_readable_callback_configuration(const GetFrameReadableCallbackConfiguration *data, GetFrameReadableCallbackConfiguration_Response *response) {
	response->header.length = sizeof(GetFrameReadableCallbackConfiguration_Response);
	response->enabled = frame_readable_callback_enabled;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_error_occurred_callback_configuration(const SetErrorOccurredCallbackConfiguration *data) {
	error_occurred_callback_enabled = data->enabled;
	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_error_occurred_callback_configuration(const GetErrorOccurredCallbackConfiguration *data, GetErrorOccurredCallbackConfiguration_Response *response) {
	response->header.length = sizeof(GetErrorOccurredCallbackConfiguration_Response);
	response->enabled = error_occurred_callback_enabled;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_timestamped_frame_configuration(const SetTimestampedFrameConfiguration *data) {
	if (data->write_backlog_size + data->read_backlog_size > TFCAN_TIMESTAMPED_BACKLOG_SIZE) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	logd("TSF %d\n\r", (int)data->enabled);

	tfcan.timestamped_frame_enabled   = data->enabled;
	tfcan.tx_timestamped_backlog_size = data->write_backlog_size;
	tfcan.rx_timestamped_backlog_size = data->read_backlog_size;
	tfcan.reconfigure_queues          = true;

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_timestamped_frame_configuration(const GetTimestampedFrameConfiguration *data, GetTimestampedFrameConfiguration_Response *response) {
	response->header.length      = sizeof(GetTimestampedFrameConfiguration_Response);
	response->enabled            = tfcan.timestamped_frame_enabled;
	response->write_backlog_size = tfcan.tx_timestamped_backlog_size;
	response->read_backlog_size  = tfcan.rx_timestamped_backlog_size;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse write_timestamped_frame_low_level(const WriteTimestampedFrameLowLevel *data, WriteTimestampedFrameLowLevel_Response *response) {
	if (data->frame_type > CAN_V2_FRAME_TYPE_EXTENDED_REMOTE ||
	    data->data_length > 15) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	if (data->frame_type < CAN_V2_FRAME_TYPE_EXTENDED_DATA) {
		if (data->identifier >= (1u << 11)) {
			return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
		}
	} else {
		if (data->identifier >= (1u << 29)) {
			return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
		}
	}

	TFCAN_TimestampedFrame timestamped_frame;

	timestamped_frame.frame.mo_type    = data->frame_type;
	timestamped_frame.frame.identifier = data->identifier;
	memcpy(timestamped_frame.frame.data, data->data_data, MIN(data->data_length, sizeof(timestamped_frame.frame.data)));
	timestamped_frame.frame.length     = data->data_length;
	timestamped_frame.timestamp        = data->timestamp;

	response->header.length = sizeof(WriteTimestampedFrameLowLevel_Response);
	response->success       = tfcan_enqueue_timestamped_frame(&timestamped_frame);

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse read_timestamped_frame_low_level(const ReadTimestampedFrameLowLevel *data, ReadTimestampedFrameLowLevel_Response *response) {
	response->header.length = sizeof(ReadTimestampedFrameLowLevel_Response);

	if (frame_read_callback_enabled || timestamped_frame_read_callback_enabled) {
		response->success     = false;
		response->frame_type  = CAN_V2_FRAME_TYPE_STANDARD_DATA;
		response->identifier  = 0;
		response->data_length = 0;
		memset(&response->data_data, 0, sizeof(response->data_data));
		response->timestamp   = 0;
	} else {
		TFCAN_TimestampedFrame timestamped_frame;

		// Need to zero the whole frame here because tfcan_dequeue_timestamped_frame
		// will not touch it if there is no frame to be read
		memset(&timestamped_frame, 0, sizeof(TFCAN_TimestampedFrame));

		response->success     = tfcan_dequeue_timestamped_frame(&timestamped_frame);

		const uint8_t length = MIN(timestamped_frame.frame.length, 8);

		response->frame_type  = timestamped_frame.frame.mo_type;
		response->identifier  = timestamped_frame.frame.identifier;
		response->data_length = timestamped_frame.frame.length;
		memcpy(response->data_data, timestamped_frame.frame.data, length);
		memset(&response->data_data[length], 0, sizeof(response->data_data) - length);
		response->timestamp   = timestamped_frame.timestamp;
	}

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse get_timestamp(const GetTimestamp *data, GetTimestamp_Response *response) {
	response->header.length = sizeof(GetTimestamp_Response);
	response->timestamp     = system_timer_get_us();

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_timestamped_frame_read_callback_configuration(const SetTimestampedFrameReadCallbackConfiguration *data) {
	timestamped_frame_read_callback_enabled = data->enabled;
	if (data->enabled) {
		frame_readable_callback_enabled = false;
	}

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_timestamped_frame_read_callback_configuration(const GetTimestampedFrameReadCallbackConfiguration *data, GetTimestampedFrameReadCallbackConfiguration_Response *response) {
	response->header.length = sizeof(GetTimestampedFrameReadCallbackConfiguration_Response);
	response->enabled       = timestamped_frame_read_callback_enabled;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

bool handle_frame_read_low_level_callback(void) {
	static bool is_buffered = false;
	static FrameReadLowLevel_Callback cb;

	if (!is_buffered) {
		if (!frame_read_callback_enabled) {
			return false;
		}

		TFCAN_Frame frame;

		if (!tfcan_dequeue_frame(&frame)) {
			return false;
		}

		const uint8_t length = MIN(frame.length, 8);

		tfp_make_default_header(&cb.header, bootloader_get_uid(), sizeof(FrameReadLowLevel_Callback), FID_CALLBACK_FRAME_READ_LOW_LEVEL);

		cb.frame_type  = frame.mo_type;
		cb.identifier  = frame.identifier;
		cb.data_length = frame.length;
		memcpy(cb.data_data, frame.data, length);
		memset(&cb.data_data[length], 0, sizeof(cb.data_data) - length);
	}

	if (bootloader_spitfp_is_send_possible(&bootloader_status.st)) {
		bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&cb, sizeof(FrameReadLowLevel_Callback));
		is_buffered = false;
		return true;
	} else {
		is_buffered = true;
	}

	return false;
}

bool handle_frame_readable_callback(void) {
	static bool is_buffered = false;
	static FrameReadable_Callback cb;

	if(!is_buffered) {
		if (!frame_readable_callback_enabled || frame_readable_callback_already_sent) {
			return false;
		}

		if (!tfcan_frame_readable()) {
			return false;
		}

		frame_readable_callback_already_sent = true;

		tfp_make_default_header(&cb.header, bootloader_get_uid(), sizeof(FrameReadable_Callback), FID_CALLBACK_FRAME_READABLE);
	}

	if(bootloader_spitfp_is_send_possible(&bootloader_status.st)) {
		bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&cb, sizeof(FrameReadable_Callback));
		is_buffered = false;
		return true;
	} else {
		is_buffered = true;
	}

	return false;
}

bool handle_timestamped_frame_read_low_level_callback(void) {
	static bool is_buffered = false;
	static TimestampedFrameReadLowLevel_Callback cb;

	if (!is_buffered) {
		if (!timestamped_frame_read_callback_enabled) {
			return false;
		}

		TFCAN_TimestampedFrame timestamped_frame;

		if (!tfcan_dequeue_timestamped_frame(&timestamped_frame)) {
			return false;
		}

		const uint8_t length = MIN(timestamped_frame.frame.length, 8);

		tfp_make_default_header(&cb.header, bootloader_get_uid(), sizeof(TimestampedFrameReadLowLevel_Callback), FID_CALLBACK_TIMESTAMPED_FRAME_READ_LOW_LEVEL);

		cb.frame_type  = timestamped_frame.frame.mo_type;
		cb.identifier  = timestamped_frame.frame.identifier;
		cb.data_length = timestamped_frame.frame.length;
		memcpy(cb.data_data, timestamped_frame.frame.data, length);
		memset(&cb.data_data[length], 0, sizeof(cb.data_data) - length);
		cb.timestamp   = timestamped_frame.timestamp;
	}

	if (bootloader_spitfp_is_send_possible(&bootloader_status.st)) {
		bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&cb, sizeof(TimestampedFrameReadLowLevel_Callback));
		is_buffered = false;
		return true;
	} else {
		is_buffered = true;
	}

	return false;
}

bool handle_error_occurred_callback(void) {
	static bool is_buffered = false;
	static ErrorOccurred_Callback cb;

	if(!is_buffered) {
		if (!error_occurred_callback_enabled) {
			return false;
		}

		if (tfcan.error_state != TFCAN_ERROR_STATE_ERROR_OCCURRED) {
			return false;
		}

		tfcan.error_state = TFCAN_ERROR_STATE_ERROR_REPORTED;

		tfp_make_default_header(&cb.header, bootloader_get_uid(), sizeof(ErrorOccurred_Callback), FID_CALLBACK_ERROR_OCCURRED);
	}

	if(bootloader_spitfp_is_send_possible(&bootloader_status.st)) {
		bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&cb, sizeof(ErrorOccurred_Callback));
		is_buffered = false;
		return true;
	} else {
		is_buffered = true;
	}

	return false;
}

void communication_tick(void) {
	communication_callback_tick();
}

void communication_init(void) {
	communication_callback_init();
}
