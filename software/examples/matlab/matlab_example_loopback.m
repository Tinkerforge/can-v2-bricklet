function matlab_example_loopback()
    import com.tinkerforge.IPConnection;
    import com.tinkerforge.BrickletCANV2;

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

    % Write standard data frame with identifier 1742 and 3 bytes of data
    data = [42, 23, 17];
    can.writeFrame(BrickletCANV2.FRAME_TYPE_STANDARD_DATA, 1742, data);

    input('Press key to exit\n', 's');
    can.setFrameReadCallbackConfiguration(false);
    ipcon.disconnect();
end

% Callback function for frame read callback
function cb_frame_read(e)
    fprintf('Frame Type: %d\n', e.frameType);
    fprintf('Identifier: %d\n', e.identifier);
    fprintf('Data (Length: %d):', e.data.length);

    for i = 1:min(e.data.length, 8)
        fprintf(' %d', e.data(i));
    end

    fprintf('\n');
    fprintf('\n');
end
