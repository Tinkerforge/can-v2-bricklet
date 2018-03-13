#!/usr/bin/env python3
# -*- coding: utf-8 -*-

HOST = "localhost"
PORT = 4223
UID = "2hghdk" # Change XYZ to the UID of your CAN Bricklet

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_can_v2 import BrickletCANV2

def cb_frame_read(frame_type, identifier, data):
    print(frame_type, identifier, data)

if __name__ == "__main__":
    ipcon = IPConnection() # Create IP connection
    can = BrickletCANV2(UID, ipcon) # Create device object

    ipcon.connect(HOST, PORT) # Connect to brickd
    # Don't use device before ipcon is connected

    can.register_callback(can.CALLBACK_FRAME_READ, cb_frame_read)
    can.set_frame_read_callback_configuration(True)

    raw_input("Press key to exit\n")
    can.set_frame_read_callback_configuration(False)
    ipcon.disconnect()
