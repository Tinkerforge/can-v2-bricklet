function matlab_example_loopback()
    import com.tinkerforge.IPConnection;
    import com.tinkerforge.BrickletCANV2;

    % FIXME: This example is incomplete

    HOST = 'localhost';
    PORT = 4223;
    UID = 'XYZ'; % Change XYZ to the UID of your CAN Bricklet 2.0

    ipcon = IPConnection(); % Create IP connection
    can = handle(BrickletCANV2(UID, ipcon), 'CallbackProperties'); % Create device object

    ipcon.connect(HOST, PORT); % Connect to brickd
    % Don't use device before ipcon is connected

    % Configure transceiver for loopback mode
    can.setTransceiverConfiguration(1000000, 625, ...
                                    BrickletCANV2.TRANSCEIVER_MODE_LOOPBACK);

    % Register frame read callback to function cb_frame_read
    set(can, 'FrameReadCallback', @(h, e) cb_frame_read(e));

    % Enable frame read callback
    can.setFrameReadCallbackConfiguration(true);

    input('Press key to exit\n', 's');
    can.setFrameReadCallbackConfiguration(false);
    ipcon.disconnect();
end

% Callback function for frame read callback
function cb_frame_read(e)
    fprintf('Frame Type: %i\n', e.frameType);
    fprintf('Identifier: %i\n', e.identifier);
    fprintf('\n');
end
