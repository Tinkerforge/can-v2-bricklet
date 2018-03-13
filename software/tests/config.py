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

    print(can.get_transceiver_configuration())
    can.set_transceiver_configuration(500000, can.TRANSCEIVER_MODE_NORMAL)
    print(can.get_transceiver_configuration())

    ipcon.disconnect()
