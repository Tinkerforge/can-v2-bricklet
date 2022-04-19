// This example is not self-contained.
// It requres usage of the example driver specific to your platform.
// See the HAL documentation.

#include "bindings/hal_common.h"
#include "bindings/bricklet_can_v2.h"

#define UID "XYZ" // Change XYZ to the UID of your CAN Bricklet 2.0

void example_setup(TF_HAL *hal);
void example_loop(TF_HAL *hal);

void check(int rc, const char* msg);

static bool frame_readable = false;
// Callback function for frame readable callback
static void frame_readable_handler(TF_CANV2 *device, void *user_data) {
	(void)device; (void)user_data; // avoid unused parameter warning

	frame_readable = true;
}

static TF_CANV2 can;

void example_setup(TF_HAL *hal) {
	// Create device object
	check(tf_can_v2_create(&can, UID, hal), "create device object");

	// Configure transceiver for loopback mode
	check(tf_can_v2_set_transceiver_configuration(&can, 1000000, 625,
	                                              TF_CAN_V2_TRANSCEIVER_MODE_LOOPBACK), "call set_transceiver_configuration");
	// Register frame readable callback to function frame_readable_handler
	tf_can_v2_register_frame_readable_callback(&can,
	                                           frame_readable_handler,
	                                           NULL);
	// Enable frame readable callback
	check(tf_can_v2_set_frame_readable_callback_configuration(&can,
	                                                          true), "call set_frame_readable_callback_configuration");
	// Write standard data frame with identifier 1742 and 3 bytes of data
	uint8_t data[3] = {42, 23, 17};
	bool success;
	check(tf_can_v2_write_frame(&can, TF_CAN_V2_FRAME_TYPE_STANDARD_DATA, 1742,
	                            data, 3, &success), "call write_frame");
	if(!success)
		tf_hal_printf("Failed to write frame.\n");
}

void example_loop(TF_HAL *hal) {
	// Poll for callbacks
	tf_hal_callback_tick(hal, 0);

	if(!frame_readable)
		return;

	frame_readable = false;

	bool success;
	uint8_t frame_type;
	uint32_t identifier;
	uint8_t data[15];
	uint8_t data_length;

	// Frame readable will only trigger once, even if there are multiple frames readable, so we use a loop to
	// read frames until there are none left.
	check(tf_can_v2_read_frame(&can, &success, &frame_type, &identifier, data, &data_length), "read frame");
	while (success) {
		if(frame_type == TF_CAN_V2_FRAME_TYPE_STANDARD_DATA) {
			tf_hal_printf("Frame Type: Standard Data\n");
		} else if(frame_type == TF_CAN_V2_FRAME_TYPE_STANDARD_REMOTE) {
			tf_hal_printf("Frame Type: Standard Remote\n");
		} else if(frame_type == TF_CAN_V2_FRAME_TYPE_EXTENDED_DATA) {
			tf_hal_printf("Frame Type: Extended Data\n");
		} else if(frame_type == TF_CAN_V2_FRAME_TYPE_EXTENDED_REMOTE) {
			tf_hal_printf("Frame Type: Extended Remote\n");
		}

		tf_hal_printf("Identifier: %I32u\n", identifier);
		tf_hal_printf("Data (Length: %I8d):", data_length);

		uint8_t i;
		for (i = 0; i < data_length && i < 8; ++i) {
			tf_hal_printf(" %I8d", data[i]);
		}

		tf_hal_printf("\n");
		check(tf_can_v2_read_frame(&can, &success, &frame_type, &identifier, data, &data_length), "read frame");
	}
}
