using System;
using Tinkerforge;

// FIXME: This example is incomplete

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
		Console.WriteLine("");
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

		Console.WriteLine("Press enter to exit");
		Console.ReadLine();
		can.SetFrameReadCallbackConfiguration(false);
		ipcon.Disconnect();
	}
}
