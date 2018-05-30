#!/usr/bin/env python
# -*- coding: utf-8 -*-

# FIXME: This example is incomplete

HOST = "localhost"
PORT = 4223
UID = "XYZ" # Change XYZ to the UID of your CAN Bricklet 2.0

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_can_v2 import BrickletCANV2

# Callback function for frame read callback
def cb_frame_read(frame_type, identifier, data):
    print("Frame Type: " + str(frame_type))
    print("Identifier: " + str(identifier))
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

    raw_input("Press key to exit\n") # Use input() in Python 3
    can.set_frame_read_callback_configuration(False)
    ipcon.disconnect()
