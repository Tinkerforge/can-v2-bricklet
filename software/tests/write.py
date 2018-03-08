#!/usr/bin/env python3
# -*- coding: utf-8 -*-

HOST = "localhost"
PORT = 4223
UID = "2hghdk" # Change XYZ to the UID of your CAN Bricklet

import time

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_can_v2 import BrickletCANV2

if __name__ == "__main__":
    ipcon = IPConnection() # Create IP connection
    can = BrickletCANV2(UID, ipcon) # Create device object

    ipcon.connect(HOST, PORT) # Connect to brickd
    # Don't use device before ipcon is connected

    if False:
        for i in range(50):
            if not can.write_frame(can.FRAME_TYPE_STANDARD_DATA, 0x321, [1, 2, 3, 4, 5, 6, 7, 8]):
                print(i, 'failed')

    can.write_frame(can.FRAME_TYPE_STANDARD_DATA, 0x321, [1, 2, 3, 4, 5, 6, 7, 8])
    can.write_frame(can.FRAME_TYPE_EXTENDED_REMOTE, (((1 << 11) - 1) << 18) + 1, [1, 2, 3, 4, 5])

    ipcon.disconnect()
