import com.tinkerforge.IPConnection;
import com.tinkerforge.BrickletCANV2;

public class ExampleLoopback {
	private static final String HOST = "localhost";
	private static final int PORT = 4223;

	// Change XYZ to the UID of your CAN Bricklet 2.0
	private static final String UID = "XYZ";

	// Note: To make the example code cleaner we do not handle exceptions. Exceptions
	//       you might normally want to catch are described in the documentation
	public static void main(String args[]) throws Exception {
		IPConnection ipcon = new IPConnection(); // Create IP connection
		BrickletCANV2 can = new BrickletCANV2(UID, ipcon); // Create device object

		ipcon.connect(HOST, PORT); // Connect to brickd
		// Don't use device before ipcon is connected

		// Configure transceiver for loopback mode
		can.setTransceiverConfiguration(1000000, 625,
		                                BrickletCANV2.TRANSCEIVER_MODE_LOOPBACK);

		// Add frame read listener
		can.addFrameReadListener(new BrickletCANV2.FrameReadListener() {
			public void frameRead(int frameType, long identifier, int[] data) {
				System.out.println("Frame Type: " + frameType);
				System.out.println("Identifier: " + identifier);

				String str = "Data (Length: " + data.length + "):";

				for(byte d : data) {
					str += " " + d;
				}

				System.out.println(str);
			}
		});

		// Enable frame read callback
		can.setFrameReadCallbackConfiguration(true);

		// Write standard data frame with identifier 1742 and 3 bytes of data
		short[] data = new short[3]{42, 23, 17};
		can.writeFrame(BrickletCAN.FRAME_TYPE_STANDARD_DATA, (short)1742, data);

		System.out.println("Press key to exit"); System.in.read();
		can.setFrameReadCallbackConfiguration(false);
		ipcon.disconnect();
	}
}
