var Tinkerforge = require('tinkerforge');

var HOST = 'localhost';
var PORT = 4223;
var UID = 'XYZ'; // Change XYZ to the UID of your CAN Bricklet 2.0

var ipcon = new Tinkerforge.IPConnection(); // Create IP connection
var can = new Tinkerforge.BrickletCANV2(UID, ipcon); // Create device object

ipcon.connect(HOST, PORT,
    function (error) {
        console.log('Error: ' + error);
    }
); // Connect to brickd
// Don't use device before ipcon is connected

ipcon.on(Tinkerforge.IPConnection.CALLBACK_CONNECTED,
    function (connectReason) {
        // Configure transceiver for loopback mode
        can.setTransceiverConfiguration(1000000, 625,
                                        Tinkerforge.BrickletCANV2.TRANSCEIVER_MODE_LOOPBACK);

        // Enable frame read callback
        can.setFrameReadCallbackConfiguration(true);

        // Write standard data frame with identifier 1742 and 3 bytes of data
        can.writeFrame(Tinkerforge.BrickletCANV2.FRAME_TYPE_STANDARD_DATA, 1742,
                       [42, 23, 17]);
    }
);

// Register frame read callback
can.on(Tinkerforge.BrickletCANV2.CALLBACK_FRAME_READ,
    // Callback function for frame read callback
    function (frameType, identifier, data) {
        console.log('Frame Type: ' + frameType);
        console.log('Identifier: ' + identifier);

        var str = 'Data (Length: ' + data.length  + '):';
        for (var i = 0; i < data.length  && i < 8; ++i) {
            str += ' ' + data[i];
        }

        console.log(str);
        console.log();
    }
);

console.log('Press key to exit');
process.stdin.on('data',
    function (data) {
        can.setFrameReadCallbackConfiguration(false);
        ipcon.disconnect();
        process.exit(0);
    }
);
