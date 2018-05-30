#!/bin/sh
# Connects to localhost:4223 by default, use --host and --port to change this

# FIXME: This example is incomplete

uid=XYZ # Change XYZ to the UID of your CAN Bricklet 2.0

# Configure transceiver for loopback mode
tinkerforge call can-v2-bricklet $uid set-transceiver-configuration 1000000 625 transceiver-mode-loopback

# Handle incoming frame read callbacks
tinkerforge dispatch can-v2-bricklet $uid frame-read &

# Enable frame read callback
tinkerforge call can-v2-bricklet $uid set-frame-read-callback-configuration true

echo "Press key to exit"; read dummy

tinkerforge call can-v2-bricklet $uid set-frame-read-callback-configuration false

kill -- -$$ # Stop callback dispatch in background
