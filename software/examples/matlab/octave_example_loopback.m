function octave_example_loopback()
    more off;

    % FIXME: This example is incomplete

    HOST = "localhost";
    PORT = 4223;
    UID = "XYZ"; % Change XYZ to the UID of your CAN Bricklet 2.0

    ipcon = javaObject("com.tinkerforge.IPConnection"); % Create IP connection
    can = javaObject("com.tinkerforge.BrickletCANV2", UID, ipcon); % Create device object

    ipcon.connect(HOST, PORT); % Connect to brickd
    % Don't use device before ipcon is connected

    % Configure transceiver for loopback mode
    can.setTransceiverConfiguration(1000000, 625, can.TRANSCEIVER_MODE_LOOPBACK);

    % Register frame read callback to function cb_frame_read
    can.addFrameReadCallback(@cb_frame_read);

    % Enable frame read callback
    can.setFrameReadCallbackConfiguration(true);

    input("Press key to exit\n", "s");
    can.setFrameReadCallbackConfiguration(false);
    ipcon.disconnect();
end

% Callback function for frame read callback
function cb_frame_read(e)
    fprintf("Frame Type: %d\n", e.frameType);
    fprintf("Identifier: %d\n", java2int(e.identifier));
    fprintf("\n");
end

function int = java2int(value)
    if compare_versions(version(), "3.8", "<=")
        int = value.intValue();
    else
        int = value;
    end
end
