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
#include "bricklib2/protocols/tfp/tfp.h"

#include "tfcan.h"

BootloaderHandleMessageResponse handle_message(const void *message, void *response) {
	switch(tfp_get_fid_from_message(message)) {
		case FID_WRITE_FRAME: return write_frame(message, response);
		case FID_READ_FRAME: return read_frame(message, response);
		case FID_ENABLE_FRAME_READ_CALLBACK: return enable_frame_read_callback(message);
		case FID_DISABLE_FRAME_READ_CALLBACK: return disable_frame_read_callback(message);
		case FID_IS_FRAME_READ_CALLBACK_ENABLED: return is_frame_read_callback_enabled(message, response);
		case FID_SET_CONFIGURATION: return set_configuration(message);
		case FID_GET_CONFIGURATION: return get_configuration(message, response);
		case FID_SET_READ_FILTER: return set_read_filter(message);
		case FID_GET_READ_FILTER: return get_read_filter(message, response);
		case FID_GET_ERROR_LOG: return get_error_log(message, response);
		default: return HANDLE_MESSAGE_RESPONSE_NOT_SUPPORTED;
	}
}

BootloaderHandleMessageResponse write_frame(const WriteFrame *data, WriteFrame_Response *response) {
	// FIXME: add parameter check

	TFCAN_Frame frame;

	if (data->frame_type == CAN_V2_FRAME_TYPE_STANDARD_DATA) {
		frame.type = TFCAN_TYPE_STANDARD;
	} else {
		frame.type = TFCAN_TYPE_EXTENDED;
	}

	frame.identifier = data->identifier;
	memcpy(frame.data, data->data, data->length);
	frame.length = data->length;

	response->header.length = sizeof(WriteFrame_Response);
	response->success       = tfcan_enqueue_frame(&frame);

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse read_frame(const ReadFrame *data, ReadFrame_Response *response) {
	response->header.length = sizeof(ReadFrame_Response);

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse enable_frame_read_callback(const EnableFrameReadCallback *data) {

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse disable_frame_read_callback(const DisableFrameReadCallback *data) {

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse is_frame_read_callback_enabled(const IsFrameReadCallbackEnabled *data, IsFrameReadCallbackEnabled_Response *response) {
	response->header.length = sizeof(IsFrameReadCallbackEnabled_Response);

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_configuration(const SetConfiguration *data) {

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_configuration(const GetConfiguration *data, GetConfiguration_Response *response) {
	response->header.length = sizeof(GetConfiguration_Response);

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_read_filter(const SetReadFilter *data) {

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_read_filter(const GetReadFilter *data, GetReadFilter_Response *response) {
	response->header.length = sizeof(GetReadFilter_Response);

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse get_error_log(const GetErrorLog *data, GetErrorLog_Response *response) {
	response->header.length = sizeof(GetErrorLog_Response);

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

bool handle_frame_read_callback(void) {
	static bool is_buffered = false;
	static FrameRead_Callback cb;

	if(!is_buffered) {
		tfp_make_default_header(&cb.header, bootloader_get_uid(), sizeof(FrameRead_Callback), FID_CALLBACK_FRAME_READ);
		// TODO: Implement FrameRead callback handling

		return false;
	}

	if(bootloader_spitfp_is_send_possible(&bootloader_status.st)) {
		bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&cb, sizeof(FrameRead_Callback));
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
