function octave_example_loopback()
    more off;

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

    % Write standard data frame with identifier 1742 and 3 bytes of data
    can.writeFrame(can.FRAME_TYPE_STANDARD_DATA, 1742, [42 23 17]);

    input("Press key to exit\n", "s");

    can.setFrameReadCallbackConfiguration(false);

    ipcon.disconnect();
end

% Callback function for frame read callback
function cb_frame_read(e)
    if e.frameType == 0
        fprintf("Frame Type: Standard Data\n");
    elseif e.frameType == 1
        fprintf("Frame Type: Standard Remote\n");
    elseif e.frameType == 2
        fprintf("Frame Type: Extended Data\n");
    elseif e.frameType == 3
        fprintf("Frame Type: Extended Remote\n");
    end

    fprintf("Identifier: %d\n", java2int(e.identifier));
    fprintf("Data (Length: %d):", e.data.length);

    for i = 1:min(e.data.length, 8)
        fprintf(" %d", java2int(e.data(i)));
    end

    fprintf("\n");
    fprintf("\n");
end

function int = java2int(value)
    if compare_versions(version(), "3.8", "<=")
        int = value.intValue();
    else
        int = value;
    end
end
