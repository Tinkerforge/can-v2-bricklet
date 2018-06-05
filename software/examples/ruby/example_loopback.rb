#!/usr/bin/env ruby
# -*- ruby encoding: utf-8 -*-

require 'tinkerforge/ip_connection'
require 'tinkerforge/bricklet_can_v2'

include Tinkerforge

HOST = 'localhost'
PORT = 4223
UID = 'XYZ' # Change XYZ to the UID of your CAN Bricklet 2.0

ipcon = IPConnection.new # Create IP connection
can = BrickletCANV2.new UID, ipcon # Create device object

ipcon.connect HOST, PORT # Connect to brickd
# Don't use device before ipcon is connected

# Configure transceiver for loopback mode
can.set_transceiver_configuration 1000000, 625, BrickletCANV2::TRANSCEIVER_MODE_LOOPBACK

# Register frame read callback
can.register_callback(BrickletCANV2::CALLBACK_FRAME_READ) do |frame_type, identifier,
                                                              data|
  puts "Frame Type: #{frame_type}"
  puts "Identifier: #{identifier}"
  puts "Data (Length: #{data.length}): #{data.join(' ')}"
end

# Enable frame read callback
can.set_frame_read_callback_configuration true

# Write standard data frame with identifier 1742 and 3 bytes of data
can.write_frame BrickletCANV2::FRAME_TYPE_STANDARD_DATA, 1742, [42, 23, 17]

puts 'Press key to exit'
$stdin.gets
can.set_frame_read_callback_configuration false
ipcon.disconnect
