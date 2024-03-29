Imports System
Imports Tinkerforge

Module ExampleLoopback
    Const HOST As String = "localhost"
    Const PORT As Integer = 4223
    Const UID As String = "XYZ" ' Change XYZ to the UID of your CAN Bricklet 2.0

    ' Callback subroutine for frame read callback
    Sub FrameReadCB(ByVal sender As BrickletCANV2, ByVal frameType As Byte, _
                    ByVal identifier As Long, ByVal data As Byte())
        If frameType = BrickletCANV2.FRAME_TYPE_STANDARD_DATA Then
            Console.WriteLine("Frame Type: Standard Data")
        Else If frameType = BrickletCANV2.FRAME_TYPE_STANDARD_REMOTE Then
            Console.WriteLine("Frame Type: Standard Remote")
        Else If frameType = BrickletCANV2.FRAME_TYPE_EXTENDED_DATA Then
            Console.WriteLine("Frame Type: Extended Data")
        Else If frameType = BrickletCANV2.FRAME_TYPE_EXTENDED_REMOTE Then
            Console.WriteLine("Frame Type: Extended Remote")
        End If

        Console.WriteLine("Identifier: " + identifier.ToString())
        Console.Write("Data (Length: " + data.Length.ToString() + "):")

        Dim i As Integer
        For i = 0 To Math.Min(data.Length - 1, 7)
            Console.Write(" " + data(i).ToString())
        Next i

        Console.WriteLine("")
        Console.WriteLine("")
    End Sub

    Sub Main()
        Dim ipcon As New IPConnection() ' Create IP connection
        Dim can As New BrickletCANV2(UID, ipcon) ' Create device object

        ipcon.Connect(HOST, PORT) ' Connect to brickd
        ' Don't use device before ipcon is connected

        ' Configure transceiver for loopback mode
        can.SetTransceiverConfiguration(1000000, 625, _
                                        BrickletCANV2.TRANSCEIVER_MODE_LOOPBACK)

        ' Register frame read callback to subroutine FrameReadCB
        AddHandler can.FrameReadCallback, AddressOf FrameReadCB

        ' Enable frame read callback
        can.SetFrameReadCallbackConfiguration(True)

        ' Write standard data frame with identifier 1742 and 3 bytes of data
        can.WriteFrame(BrickletCANV2.FRAME_TYPE_STANDARD_DATA, 1742, _
                       new Byte(){42, 23, 17})

        Console.WriteLine("Press key to exit")
        Console.ReadLine()

        can.SetFrameReadCallbackConfiguration(False)

        ipcon.Disconnect()
    End Sub
End Module
