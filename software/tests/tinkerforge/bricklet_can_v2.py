# -*- coding: utf-8 -*-
#############################################################
# This file was automatically generated on 2018-03-07.      #
#                                                           #
# Python Bindings Version 2.1.16                            #
#                                                           #
# If you have a bugfix for this file and want to commit it, #
# please fix the bug in the generator. You can find a link  #
# to the generators git repository on tinkerforge.com       #
#############################################################

#### __DEVICE_IS_NOT_RELEASED__ ####

from collections import namedtuple

try:
    from .ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data
except ValueError:
    from ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data

ReadFrame = namedtuple('ReadFrame', ['success', 'frame_type', 'identifier', 'data', 'length'])
GetConfiguration = namedtuple('Configuration', ['baud_rate', 'transceiver_mode', 'write_timeout'])
GetReadFilter = namedtuple('ReadFilter', ['mode', 'mask', 'filter1', 'filter2'])
GetErrorLog = namedtuple('ErrorLog', ['write_error_level', 'read_error_level', 'transceiver_disabled', 'write_timeout_count', 'read_register_overflow_count', 'read_buffer_overflow_count'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletCANV2(Device):
    """
    Communicates with CAN bus devices
    """

    DEVICE_IDENTIFIER = 2107
    DEVICE_DISPLAY_NAME = 'CAN Bricklet 2.0'
    DEVICE_URL_PART = 'can_v2' # internal

    CALLBACK_FRAME_READ = 11


    FUNCTION_WRITE_FRAME = 1
    FUNCTION_READ_FRAME = 2
    FUNCTION_ENABLE_FRAME_READ_CALLBACK = 3
    FUNCTION_DISABLE_FRAME_READ_CALLBACK = 4
    FUNCTION_IS_FRAME_READ_CALLBACK_ENABLED = 5
    FUNCTION_SET_CONFIGURATION = 6
    FUNCTION_GET_CONFIGURATION = 7
    FUNCTION_SET_READ_FILTER = 8
    FUNCTION_GET_READ_FILTER = 9
    FUNCTION_GET_ERROR_LOG = 10
    FUNCTION_GET_SPITFP_ERROR_COUNT = 234
    FUNCTION_SET_BOOTLOADER_MODE = 235
    FUNCTION_GET_BOOTLOADER_MODE = 236
    FUNCTION_SET_WRITE_FIRMWARE_POINTER = 237
    FUNCTION_WRITE_FIRMWARE = 238
    FUNCTION_SET_STATUS_LED_CONFIG = 239
    FUNCTION_GET_STATUS_LED_CONFIG = 240
    FUNCTION_GET_CHIP_TEMPERATURE = 242
    FUNCTION_RESET = 243
    FUNCTION_WRITE_UID = 248
    FUNCTION_READ_UID = 249
    FUNCTION_GET_IDENTITY = 255

    FRAME_TYPE_STANDARD_DATA = 0
    FRAME_TYPE_STANDARD_REMOTE = 1
    FRAME_TYPE_EXTENDED_DATA = 2
    FRAME_TYPE_EXTENDED_REMOTE = 3
    BAUD_RATE_10KBPS = 0
    BAUD_RATE_20KBPS = 1
    BAUD_RATE_50KBPS = 2
    BAUD_RATE_125KBPS = 3
    BAUD_RATE_250KBPS = 4
    BAUD_RATE_500KBPS = 5
    BAUD_RATE_800KBPS = 6
    BAUD_RATE_1000KBPS = 7
    TRANSCEIVER_MODE_NORMAL = 0
    TRANSCEIVER_MODE_LOOPBACK = 1
    TRANSCEIVER_MODE_READ_ONLY = 2
    FILTER_MODE_DISABLED = 0
    FILTER_MODE_ACCEPT_ALL = 1
    FILTER_MODE_MATCH_STANDARD = 2
    FILTER_MODE_MATCH_STANDARD_AND_DATA = 3
    FILTER_MODE_MATCH_EXTENDED = 4
    BOOTLOADER_MODE_BOOTLOADER = 0
    BOOTLOADER_MODE_FIRMWARE = 1
    BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT = 2
    BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT = 3
    BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT = 4
    BOOTLOADER_STATUS_OK = 0
    BOOTLOADER_STATUS_INVALID_MODE = 1
    BOOTLOADER_STATUS_NO_CHANGE = 2
    BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT = 3
    BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT = 4
    BOOTLOADER_STATUS_CRC_MISMATCH = 5
    STATUS_LED_CONFIG_OFF = 0
    STATUS_LED_CONFIG_ON = 1
    STATUS_LED_CONFIG_SHOW_HEARTBEAT = 2
    STATUS_LED_CONFIG_SHOW_STATUS = 3

    def __init__(self, uid, ipcon):
        """
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletCANV2.FUNCTION_WRITE_FRAME] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_READ_FRAME] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_ENABLE_FRAME_READ_CALLBACK] = BrickletCANV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_DISABLE_FRAME_READ_CALLBACK] = BrickletCANV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_IS_FRAME_READ_CALLBACK_ENABLED] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_SET_CONFIGURATION] = BrickletCANV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletCANV2.FUNCTION_GET_CONFIGURATION] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_SET_READ_FILTER] = BrickletCANV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletCANV2.FUNCTION_GET_READ_FILTER] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_GET_ERROR_LOG] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_SET_BOOTLOADER_MODE] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_GET_BOOTLOADER_MODE] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletCANV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletCANV2.FUNCTION_WRITE_FIRMWARE] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletCANV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletCANV2.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_RESET] = BrickletCANV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletCANV2.FUNCTION_WRITE_UID] = BrickletCANV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletCANV2.FUNCTION_READ_UID] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_GET_IDENTITY] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletCANV2.CALLBACK_FRAME_READ] = 'B I 8B B'


    def write_frame(self, frame_type, identifier, data, length):
        """
        Writes a data or remote frame to the write buffer to be transmitted over the
        CAN transceiver.

        The Bricklet supports the standard 11-bit (CAN 2.0A) and the additional extended
        18-bit (CAN 2.0B) identifiers. For standard frames the Bricklet uses bit 0 to 10
        from the ``identifier`` parameter as standard 11-bit identifier. For extended
        frames the Bricklet additionally uses bit 11 to 28 from the ``identifier``
        parameter as extended 18-bit identifier.

        For remote frames the ``data`` parameter is ignored.

        Returns *true* if the frame was successfully added to the write buffer. Returns
        *false* if the frame could not be added because write buffer is already full.

        The write buffer can overflow if frames are written to it at a higher rate
        than the Bricklet can transmitted them over the CAN transceiver. This may
        happen if the CAN transceiver is configured as read-only or is using a low baud
        rate (see :func:`Set Configuration`). It can also happen if the CAN bus is
        congested and the frame cannot be transmitted because it constantly loses
        arbitration or because the CAN transceiver is currently disabled due to a high
        write error level (see :func:`Get Error Log`).
        """
        frame_type = int(frame_type)
        identifier = int(identifier)
        data = list(map(int, data))
        length = int(length)

        return self.ipcon.send_request(self, BrickletCANV2.FUNCTION_WRITE_FRAME, (frame_type, identifier, data, length), 'B I 8B B', '!')

    def read_frame(self):
        """
        Tries to read the next data or remote frame from the read buffer and return it.
        If a frame was successfully read, then the ``success`` return value is set to
        *true* and the other return values contain the frame. If the read buffer is
        empty and no frame could be read, then the ``success`` return value is set to
        *false* and the other return values contain invalid data.

        The ``identifier`` return value follows the identifier format described for
        :func:`Write Frame`.

        For remote frames the ``data`` return value always contains invalid data.

        A configurable read filter can be used to define which frames should be
        received by the CAN transceiver and put into the read buffer (see
        :func:`Set Read Filter`).

        Instead of polling with this function, you can also use callbacks. See the
        :func:`Enable Frame Read Callback` function and the :cb:`Frame Read` callback.
        """
        return ReadFrame(*self.ipcon.send_request(self, BrickletCANV2.FUNCTION_READ_FRAME, (), '', '! B I 8B B'))

    def enable_frame_read_callback(self):
        """
        Enables the :cb:`Frame Read` callback.

        By default the callback is disabled.
        """
        self.ipcon.send_request(self, BrickletCANV2.FUNCTION_ENABLE_FRAME_READ_CALLBACK, (), '', '')

    def disable_frame_read_callback(self):
        """
        Disables the :cb:`Frame Read` callback.

        By default the callback is disabled.
        """
        self.ipcon.send_request(self, BrickletCANV2.FUNCTION_DISABLE_FRAME_READ_CALLBACK, (), '', '')

    def is_frame_read_callback_enabled(self):
        """
        Returns *true* if the :cb:`Frame Read` callback is enabled, *false* otherwise.
        """
        return self.ipcon.send_request(self, BrickletCANV2.FUNCTION_IS_FRAME_READ_CALLBACK_ENABLED, (), '', '!')

    def set_configuration(self, baud_rate, transceiver_mode, write_timeout):
        """
        Sets the configuration for the CAN bus communication.

        The baud rate can be configured in steps between 10 and 1000 kbit/s.

        The CAN transceiver has three different modes:

        * Normal: Reads from and writes to the CAN bus and performs active bus
          error detection and acknowledgement.
        * Loopback: All reads and writes are performed internally. The transceiver
          is disconnected from the actual CAN bus.
        * Read-Only: Only reads from the CAN bus, but does neither active bus error
          detection nor acknowledgement. Only the receiving part of the transceiver
          is connected to the CAN bus.

        The write timeout has three different modes that define how a failed frame
        transmission should be handled:

        * One-Shot (< 0): Only one transmission attempt will be made. If the
          transmission fails then the frame is discarded.
        * Infinite (= 0): Infinite transmission attempts will be made. The frame will
          never be discarded.
        * Milliseconds (> 0): A limited number of transmission attempts will be made.
          If the frame could not be transmitted successfully after the configured
          number of milliseconds then the frame is discarded.

        The default is: 125 kbit/s, normal transceiver mode and infinite write timeout.
        """
        baud_rate = int(baud_rate)
        transceiver_mode = int(transceiver_mode)
        write_timeout = int(write_timeout)

        self.ipcon.send_request(self, BrickletCANV2.FUNCTION_SET_CONFIGURATION, (baud_rate, transceiver_mode, write_timeout), 'B B i', '')

    def get_configuration(self):
        """
        Returns the configuration as set by :func:`Set Configuration`.
        """
        return GetConfiguration(*self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_CONFIGURATION, (), '', 'B B i'))

    def set_read_filter(self, mode, mask, filter1, filter2):
        """
        Set the read filter configuration. This can be used to define which frames
        should be received by the CAN transceiver and put into the read buffer.

        The read filter has five different modes that define if and how the mask and
        the two filters are applied:

        * Disabled: No filtering is applied at all. All frames are received even
          incomplete and defective frames. This mode should be used for debugging only.
        * Accept-All: All complete and error-free frames are received.
        * Match-Standard: Only standard frames with a matching identifier are received.
        * Match-Standard-and-Data: Only standard frames with matching identifier and
          data bytes are received.
        * Match-Extended: Only extended frames with a matching identifier are received.

        The mask and filters are used as bit masks. Their usage depends on the mode:

        * Disabled: Mask and filters are ignored.
        * Accept-All: Mask and filters are ignored.
        * Match-Standard: Bit 0 to 10 (11 bits) of mask and filters are used to match
          the 11-bit identifier of standard frames.
        * Match-Standard-and-Data: Bit 0 to 10 (11 bits) of mask and filters are used
          to match the 11-bit identifier of standard frames. Bit 11 to 18 (8 bits) and
          bit 19 to 26 (8 bits) of mask and filters are used to match the first and
          second data byte (if present) of standard frames.
        * Match-Extended: Bit 0 to 10 (11 bits) of mask and filters are used
          to match the standard 11-bit identifier part of extended frames. Bit 11 to 28
          (18 bits) of mask and filters are used to match the extended 18-bit identifier
          part of extended frames.

        The mask and filters are applied in this way: The mask is used to select the
        identifier and data bits that should be compared to the corresponding filter
        bits. All unselected bits are automatically accepted. All selected bits have
        to match one of the filters to be accepted. If all bits for the selected mode
        are accepted then the frame is accepted and is added to the read buffer.

        .. csv-table::
         :header: "Mask Bit", "Filter Bit", "Identifier/Data Bit", "Result"
         :widths: 10, 10, 10, 10

         0, X, X, Accept
         1, 0, 0, Accept
         1, 0, 1, Reject
         1, 1, 0, Reject
         1, 1, 1, Accept

        For example, to receive standard frames with identifier 0x123 only the mode can
        be set to Match-Standard with 0x7FF as mask and 0x123 as filter 1 and filter 2.
        The mask of 0x7FF selects all 11 identifier bits for matching so that the
        identifier has to be exactly 0x123 to be accepted.

        To accept identifier 0x123 and identifier 0x456 at the same time, just set
        filter 2 to 0x456 and keep mask and filter 1 unchanged.

        The default mode is accept-all.
        """
        mode = int(mode)
        mask = int(mask)
        filter1 = int(filter1)
        filter2 = int(filter2)

        self.ipcon.send_request(self, BrickletCANV2.FUNCTION_SET_READ_FILTER, (mode, mask, filter1, filter2), 'B I I I', '')

    def get_read_filter(self):
        """
        Returns the read filter as set by :func:`Set Read Filter`.
        """
        return GetReadFilter(*self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_READ_FILTER, (), '', 'B I I I'))

    def get_error_log(self):
        """
        Returns information about different kinds of errors.

        The write and read error levels indicate the current level of checksum,
        acknowledgement, form, bit and stuffing errors during CAN bus write and read
        operations.

        When the write error level extends 255 then the CAN transceiver gets disabled
        and no frames can be transmitted or received anymore. The CAN transceiver will
        automatically be activated again after the CAN bus is idle for a while.

        The write and read error levels are not available in read-only transceiver mode
        (see :func:`Set Configuration`) and are reset to 0 as a side effect of changing
        the configuration or the read filter.

        The write timeout, read register and buffer overflow counts represents the
        number of these errors:

        * A write timeout occurs if a frame could not be transmitted before the
          configured write timeout expired (see :func:`Set Configuration`).
        * A read register overflow occurs if the read register of the CAN transceiver
          still contains the last received frame when the next frame arrives. In this
          case the newly arrived frame is lost. This happens if the CAN transceiver
          receives more frames than the Bricklet can handle. Using the read filter
          (see :func:`Set Read Filter`) can help to reduce the amount of received frames.
          This count is not exact, but a lower bound, because the Bricklet might not
          able detect all overflows if they occur in rapid succession.
        * A read buffer overflow occurs if the read buffer of the Bricklet is already
          full when the next frame should be read from the read register of the CAN
          transceiver. In this case the frame in the read register is lost. This
          happens if the CAN transceiver receives more frames to be added to the read
          buffer than are removed from the read buffer using the :func:`Read Frame`
          function. Using the :cb:`Frame Read` callback ensures that the read buffer
          can not overflow.
        """
        return GetErrorLog(*self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_ERROR_LOG, (), '', 'B B ! I I I'))

    def get_spitfp_error_count(self):
        """
        Returns the error count for the communication between Brick and Bricklet.

        The errors are divided into

        * ack checksum errors,
        * message checksum errors,
        * frameing errors and
        * overflow errors.

        The errors counts are for errors that occur on the Bricklet side. All
        Bricks have a similar function that returns the errors on the Brick side.
        """
        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 'I I I I'))

    def set_bootloader_mode(self, mode):
        """
        Sets the bootloader mode and returns the status after the requested
        mode change was instigated.

        You can change from bootloader mode to firmware mode and vice versa. A change
        from bootloader mode to firmware mode will only take place if the entry function,
        device identifier und crc are present and correct.

        This function is used by Brick Viewer during flashing. It should not be
        necessary to call it in a normal user program.
        """
        mode = int(mode)

        return self.ipcon.send_request(self, BrickletCANV2.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 'B')

    def get_bootloader_mode(self):
        """
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        return self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_BOOTLOADER_MODE, (), '', 'B')

    def set_write_firmware_pointer(self, pointer):
        """
        Sets the firmware pointer for :func:`Write Firmware`. The pointer has
        to be increased by chunks of size 64. The data is written to flash
        every 4 chunks (which equals to one page of size 256).

        This function is used by Brick Viewer during flashing. It should not be
        necessary to call it in a normal user program.
        """
        pointer = int(pointer)

        self.ipcon.send_request(self, BrickletCANV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', '')

    def write_firmware(self, data):
        """
        Writes 64 Bytes of firmware at the position as written by
        :func:`Set Write Firmware Pointer` before. The firmware is written
        to flash every 4 chunks.

        You can only write firmware in bootloader mode.

        This function is used by Brick Viewer during flashing. It should not be
        necessary to call it in a normal user program.
        """
        data = list(map(int, data))

        return self.ipcon.send_request(self, BrickletCANV2.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 'B')

    def set_status_led_config(self, config):
        """
        Sets the status LED configuration. By default the LED shows
        communication traffic between Brick and Bricklet, it flickers once
        for every 10 received data packets.

        You can also turn the LED permanently on/off or show a heartbeat.

        If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
        """
        config = int(config)

        self.ipcon.send_request(self, BrickletCANV2.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', '')

    def get_status_led_config(self):
        """
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        return self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 'B')

    def get_chip_temperature(self):
        """
        Returns the temperature in °C as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        return self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 'h')

    def reset(self):
        """
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.ipcon.send_request(self, BrickletCANV2.FUNCTION_RESET, (), '', '')

    def write_uid(self, uid):
        """
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        uid = int(uid)

        self.ipcon.send_request(self, BrickletCANV2.FUNCTION_WRITE_UID, (uid,), 'I', '')

    def read_uid(self):
        """
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        return self.ipcon.send_request(self, BrickletCANV2.FUNCTION_READ_UID, (), '', 'I')

    def get_identity(self):
        """
        Returns the UID, the UID where the Bricklet is connected to,
        the position, the hardware and firmware version as well as the
        device identifier.

        The position can be 'a', 'b', 'c' or 'd'.

        The device identifier numbers can be found :ref:`here <device_identifier>`.
        |device_identifier_constant|
        """
        return GetIdentity(*self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_IDENTITY, (), '', '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        """
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

CANV2 = BrickletCANV2 # for backward compatibility
