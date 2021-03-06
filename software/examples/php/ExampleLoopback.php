<?php

require_once('Tinkerforge/IPConnection.php');
require_once('Tinkerforge/BrickletCANV2.php');

use Tinkerforge\IPConnection;
use Tinkerforge\BrickletCANV2;

const HOST = 'localhost';
const PORT = 4223;
const UID = 'XYZ'; // Change XYZ to the UID of your CAN Bricklet 2.0

// Callback function for frame read callback
function cb_frameRead($frame_type, $identifier, $data)
{
    if ($frame_type == BrickletCANV2::FRAME_TYPE_STANDARD_DATA) {
        echo "Frame Type: Standard Data\n";
    } elseif ($frame_type == BrickletCANV2::FRAME_TYPE_STANDARD_REMOTE) {
        echo "Frame Type: Standard Remote\n";
    } elseif ($frame_type == BrickletCANV2::FRAME_TYPE_EXTENDED_DATA) {
        echo "Frame Type: Extended Data\n";
    } elseif ($frame_type == BrickletCANV2::FRAME_TYPE_EXTENDED_REMOTE) {
        echo "Frame Type: Extended Remote\n";
    }

    echo "Identifier: $identifier\n";
    echo "Data (Length: " . count($data) . "):";

    for ($i = 0; $i < count($data) && $i < 8; ++$i) {
        echo " " . $data[$i];
    }

    echo "\n";
    echo "\n";
}

$ipcon = new IPConnection(); // Create IP connection
$can = new BrickletCANV2(UID, $ipcon); // Create device object

$ipcon->connect(HOST, PORT); // Connect to brickd
// Don't use device before ipcon is connected

// Configure transceiver for loopback mode
$can->setTransceiverConfiguration(1000000, 625, BrickletCANV2::TRANSCEIVER_MODE_LOOPBACK);

// Register frame read callback to function cb_frameRead
$can->registerCallback(BrickletCANV2::CALLBACK_FRAME_READ, 'cb_frameRead');

// Enable frame read callback
$can->setFrameReadCallbackConfiguration(TRUE);

// Write standard data frame with identifier 1742 and 3 bytes of data
$can->writeFrame(BrickletCANV2::FRAME_TYPE_STANDARD_DATA, 1742, array(42, 23, 17));

echo "Press ctrl+c to exit\n";
$ipcon->dispatchCallbacks(-1); // Dispatch callbacks forever

?>
