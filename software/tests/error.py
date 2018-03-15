#!/usr/bin/env python3
# -*- coding: utf-8 -*-

HOST = "localhost"
PORT = 4223
UID = "2hghdk" # Change XYZ to the UID of your CAN Bricklet

from pprint import pprint
import time

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_can_v2 import BrickletCANV2

if __name__ == "__main__":
    ipcon = IPConnection() # Create IP connection
    can = BrickletCANV2(UID, ipcon) # Create device object

    ipcon.connect(HOST, PORT) # Connect to brickd
    # Don't use device before ipcon is connected

    #can.set_error_led_config(can.ERROR_LED_CONFIG_SHOW_ERROR)

    while True:
        try:
            print(time.time())

            for key, value in can.get_error_log()._asdict().items():
                print('{0}: {1}'.format(key, value))

            print('')

            time.sleep(2)
        except KeyboardInterrupt:
            break

    ipcon.disconnect()
