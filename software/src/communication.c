/* can-v2-bricklet
 * Copyright (C) 2018 Matthias Bolte <matthias@tinkerforge.com>
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

#include "tfcan.h"

extern TFCAN tfcan;

static bool frame_read_callback_enabled = false;

BootloaderHandleMessageResponse handle_message(const void *message, void *response) {
	switch(tfp_get_fid_from_message(message)) {
		case FID_WRITE_FRAME_LOW_LEVEL: return write_frame_low_level(message, response);
		case FID_READ_FRAME_LOW_LEVEL: return read_frame_low_level(message, response);
		case FID_SET_FRAME_READ_CALLBACK_CONFIGURATION: return set_frame_read_callback_configuration(message);
		case FID_GET_FRAME_READ_CALLBACK_CONFIGURATION: return get_frame_read_callback_configuration(message, response);
		case FID_SET_TRANSCEIVER_CONFIGURATION: return set_transceiver_configuration(message);
		case FID_GET_TRANSCEIVER_CONFIGURATION: return get_transceiver_configuration(message, response);
		case FID_SET_WRITE_QUEUE_CONFIGURATION: return set_write_queue_configuration(message);
		case FID_GET_WRITE_QUEUE_CONFIGURATION: return get_write_queue_configuration(message, response);
		case FID_SET_READ_QUEUE_BUFFER_CONFIGURATION: return set_read_queue_buffer_configuration(message);
		case FID_GET_READ_QUEUE_BUFFER_CONFIGURATION: return get_read_queue_buffer_configuration(message, response);
		case FID_SET_READ_QUEUE_BACKLOG_CONFIGURATION: return set_read_queue_backlog_configuration(message);
		case FID_GET_READ_QUEUE_BACKLOG_CONFIGURATION: return get_read_queue_backlog_configuration(message, response);
		case FID_GET_ERROR_LOG: return get_error_log(message, response);
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

	if (frame_read_callback_enabled) {
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

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_frame_read_callback_configuration(const GetFrameReadCallbackConfiguration *data, GetFrameReadCallbackConfiguration_Response *response) {
	response->header.length = sizeof(GetFrameReadCallbackConfiguration_Response);
	response->enabled       = frame_read_callback_enabled;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_transceiver_configuration(const SetTransceiverConfiguration *data) {
	if (data->baud_rate < 10000 || data->baud_rate > 1000000 ||
	    data->transceiver_mode > CAN_V2_TRANSCEIVER_MODE_READ_ONLY) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	tfcan.reconfigure_transceiver = true;
	tfcan.baud_rate               = data->baud_rate;
	tfcan.transceiver_mode        = data->transceiver_mode;

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_transceiver_configuration(const GetTransceiverConfiguration *data, GetTransceiverConfiguration_Response *response) {
	response->header.length    = sizeof(GetTransceiverConfiguration_Response);
	response->baud_rate        = tfcan.baud_rate;
	response->transceiver_mode = tfcan.transceiver_mode;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_write_queue_configuration(const SetWriteQueueConfiguration *data) {

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_write_queue_configuration(const GetWriteQueueConfiguration *data, GetWriteQueueConfiguration_Response *response) {
	response->header.length = sizeof(GetWriteQueueConfiguration_Response);

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_read_queue_buffer_configuration(const SetReadQueueBufferConfiguration *data) {

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_read_queue_buffer_configuration(const GetReadQueueBufferConfiguration *data, GetReadQueueBufferConfiguration_Response *response) {
	response->header.length = sizeof(GetReadQueueBufferConfiguration_Response);

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_read_queue_backlog_configuration(const SetReadQueueBacklogConfiguration *data) {

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_read_queue_backlog_configuration(const GetReadQueueBacklogConfiguration *data, GetReadQueueBacklogConfiguration_Response *response) {
	response->header.length = sizeof(GetReadQueueBacklogConfiguration_Response);

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse get_error_log(const GetErrorLog *data, GetErrorLog_Response *response) {
	response->header.length = sizeof(GetErrorLog_Response);

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

void communication_tick(void) {
	communication_callback_tick();
}

void communication_init(void) {
	communication_callback_init();
}
