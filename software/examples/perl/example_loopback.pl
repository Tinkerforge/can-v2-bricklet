#!/usr/bin/perl

# FIXME: This example is incomplete

use strict;
use Tinkerforge::IPConnection;
use Tinkerforge::BrickletCANV2;

use constant HOST => 'localhost';
use constant PORT => 4223;
use constant UID => 'XYZ'; # Change XYZ to the UID of your CAN Bricklet 2.0

# Callback subroutine for frame read callback
sub cb_frame_read
{
    my ($frame_type, $identifier, $data) = @_;

    print "Frame Type: $frame_type\n";
    print "Identifier: $identifier\n";
    print "\n";
}

my $ipcon = Tinkerforge::IPConnection->new(); # Create IP connection
my $can = Tinkerforge::BrickletCANV2->new(&UID, $ipcon); # Create device object

$ipcon->connect(&HOST, &PORT); # Connect to brickd
# Don't use device before ipcon is connected

# Configure transceiver for loopback mode
$can->set_transceiver_configuration(1000000, 625, $can->TRANSCEIVER_MODE_LOOPBACK);

# Register frame read callback to subroutine cb_frame_read
$can->register_callback($can->CALLBACK_FRAME_READ, 'cb_frame_read');

# Enable frame read callback
$can->set_frame_read_callback_configuration(1);

print "Press key to exit\n";
<STDIN>;
$can->set_frame_read_callback_configuration(0);
$ipcon->disconnect();