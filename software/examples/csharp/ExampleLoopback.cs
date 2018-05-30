using System;
using Tinkerforge;

class Example
{
	private static string HOST = "localhost";
	private static int PORT = 4223;
	private static string UID = "XYZ"; // Change XYZ to the UID of your CAN Bricklet 2.0

	// Callback function for frame read callback
	static void FrameReadCB(BrickletCANV2 sender, byte frameType, long identifier,
	                        byte[] data)
	{
		Console.WriteLine("Frame Type: " + frameType);
		Console.WriteLine("Identifier: " + identifier);

		string s = "Data (Length: " + data.Length + "):";

		foreach (byte d in data) {
			s += " " + d;
		}

		Console.WriteLine(s);
	}

	static void Main()
	{
		IPConnection ipcon = new IPConnection(); // Create IP connection
		BrickletCANV2 can = new BrickletCANV2(UID, ipcon); // Create device object

		ipcon.Connect(HOST, PORT); // Connect to brickd
		// Don't use device before ipcon is connected

		// Configure transceiver for loopback mode
		can.SetTransceiverConfiguration(1000000, 625,
		                                BrickletCANV2.TRANSCEIVER_MODE_LOOPBACK);

		// Register frame read callback to function FrameReadCB
		can.FrameReadCallback += FrameReadCB;

		// Enable frame read callback
		can.SetFrameReadCallbackConfiguration(true);

		// Write standard data frame with identifier 1742 and 3 bytes of data
		byte[] data = new byte[3]{42, 23, 17};
		can.WriteFrame(BrickletCAN.FRAME_TYPE_STANDARD_DATA, 1742, data);

		Console.WriteLine("Press enter to exit");
		Console.ReadLine();
		can.SetFrameReadCallbackConfiguration(false);
		ipcon.Disconnect();
	}
}
