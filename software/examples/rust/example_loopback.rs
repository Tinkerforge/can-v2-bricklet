use std::{error::Error, io, thread};
use tinkerforge::{can_v2_bricklet::*, ipconnection::IpConnection};

const HOST: &str = "127.0.0.1";
const PORT: u16 = 4223;
const UID: &str = "XYZ"; // Change XYZ to the UID of your CAN Bricklet 2.0

fn main() -> Result<(), Box<dyn Error>> {
    let ipcon = IpConnection::new(); // Create IP connection
    let can_v2_bricklet = CANV2Bricklet::new(UID, &ipcon); // Create device object

    ipcon.connect(HOST, PORT).recv()??; // Connect to brickd
                                        // Don't use device before ipcon is connected

    // Configure transceiver for loopback mode
    can_v2_bricklet.set_transceiver_configuration(1000000, 625, CAN_V2_BRICKLET_TRANSCEIVER_MODE_LOOPBACK);

    //Create listener for frame read events.
    let frame_read_listener = can_v2_bricklet.get_frame_read_receiver();
    // Spawn thread to handle received events. This thread ends when the can_v2_bricklet
    // is dropped, so there is no need for manual cleanup.
    thread::spawn(move || {
        for event in frame_read_listener {
            match event {
                Some((payload, result)) => {
                    println!("Frame Type: {}", result.frame_type);
                    println!("Identifier: {}", result.identifier);
                    print!("Data (Length: {}):", payload.len());
                    for item in payload.iter() {
                        print!(" {}", item);
                    }
                    println!();
                    println!();
                }
                None => println!("Stream was out of sync."),
            }
        }
    });

    // Enable frame read callback
    can_v2_bricklet.set_frame_read_callback_configuration(true);

    // Write standard data frame with identifier 1742 and 3 bytes of data
    can_v2_bricklet.write_frame(CAN_V2_BRICKLET_FRAME_TYPE_STANDARD_DATA, 1742, &[42, 23, 17])?;

    println!("Press enter to exit.");
    let mut _input = String::new();
    io::stdin().read_line(&mut _input)?;
    can_v2_bricklet.set_frame_read_callback_configuration(false);
    ipcon.disconnect();
    Ok(())
}
