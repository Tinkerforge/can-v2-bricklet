#!/usr/bin/env python
# -*- coding: utf-8 -*-

HOST = "localhost"
PORT = 4223
UID = "XYZ" # Change XYZ to the UID of your CAN Bricklet 2.0

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_can_v2 import BrickletCANV2

# Callback function for frame read callback
def cb_frame_read(frame_type, identifier, data):
    if frame_type == BrickletCANV2.FRAME_TYPE_STANDARD_DATA:
        print("Frame Type: Standard Data")
    elif frame_type == BrickletCANV2.FRAME_TYPE_STANDARD_REMOTE:
        print("Frame Type: Standard Remote")
    elif frame_type == BrickletCANV2.FRAME_TYPE_EXTENDED_DATA:
        print("Frame Type: Extended Data")
    elif frame_type == BrickletCANV2.FRAME_TYPE_EXTENDED_REMOTE:
        print("Frame Type: Extended Remote")

    print("Identifier: " + str(identifier))
    print("Data (Length: " + str(len(data)) + "): " + ", ".join(map(str, data[:min(len(data), 8)])))
    print("")

if __name__ == "__main__":
    ipcon = IPConnection() # Create IP connection
    can = BrickletCANV2(UID, ipcon) # Create device object

    ipcon.connect(HOST, PORT) # Connect to brickd
    # Don't use device before ipcon is connected

    # Configure transceiver for loopback mode
    can.set_transceiver_configuration(1000000, 625, can.TRANSCEIVER_MODE_LOOPBACK)

    # Register frame read callback to function cb_frame_read
    can.register_callback(can.CALLBACK_FRAME_READ, cb_frame_read)

    # Enable frame read callback
    can.set_frame_read_callback_configuration(True)

    # Write standard data frame with identifier 1742 and 3 bytes of data
    can.write_frame(can.FRAME_TYPE_STANDARD_DATA, 1742, [42, 23, 17])

    input("Press key to exit\n") # Use raw_input() in Python 2

    can.set_frame_read_callback_configuration(False)

    ipcon.disconnect()
