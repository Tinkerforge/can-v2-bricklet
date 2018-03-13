#!/usr/bin/env python3
# -*- coding: utf-8 -*-

HOST = "localhost"
PORT = 4223
UID = "2hghdk" # Change XYZ to the UID of your CAN Bricklet

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_can_v2 import BrickletCANV2

if __name__ == "__main__":
    ipcon = IPConnection() # Create IP connection
    can = BrickletCANV2(UID, ipcon) # Create device object

    ipcon.connect(HOST, PORT) # Connect to brickd
    # Don't use device before ipcon is connected

    while True:
        try:
            frame = can.read_frame()

            if frame.success:
                print(frame)
        except KeyboardInterrupt:
            break

    ipcon.disconnect()
