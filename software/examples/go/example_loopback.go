package main

import (
	"fmt"
	"github.com/Tinkerforge/go-api-bindings/can_v2_bricklet"
	"github.com/Tinkerforge/go-api-bindings/ipconnection"
)

const ADDR string = "localhost:4223"
const UID string = "XYZ" // Change XYZ to the UID of your CAN Bricklet 2.0.

func main() {
	ipcon := ipconnection.New()
	defer ipcon.Close()
	can, _ := can_v2_bricklet.New(UID, &ipcon) // Create device object.

	ipcon.Connect(ADDR) // Connect to brickd.
	defer ipcon.Disconnect()
	// Don't use device before ipcon is connected.

	// Configure transceiver for loopback mode
	can.SetTransceiverConfiguration(1000000, 625,
		can_v2_bricklet.TransceiverModeLoopback)

	can.RegisterFrameReadCallback(func(frameType can_v2_bricklet.FrameType, identifier uint32, data []uint8) {
		if frameType == can_v2_bricklet.FrameTypeStandardData {
			fmt.Println("Frame Type: Standard Data")
		} else if frameType == can_v2_bricklet.FrameTypeStandardRemote {
			fmt.Println("Frame Type: Standard Remote")
		} else if frameType == can_v2_bricklet.FrameTypeExtendedData {
			fmt.Println("Frame Type: Extended Data")
		} else if frameType == can_v2_bricklet.FrameTypeExtendedRemote {
			fmt.Println("Frame Type: Extended Remote")
		}

		fmt.Printf("Identifier: %d\n", identifier)
		fmt.Printf("Data (Length: %d)\n:", len(data))
		for _, item := range data {
			fmt.Printf(" %d", item)
		}
		fmt.Println()
		fmt.Println()
	})

	// Enable frame read callback
	can.SetFrameReadCallbackConfiguration(true)

	// Write standard data frame with identifier 1742 and 3 bytes of data
	can.WriteFrame(can_v2_bricklet.FrameTypeStandardData, 1742, []uint8{42, 23, 17})

	fmt.Print("Press enter to exit.")
	fmt.Scanln()

	can.SetFrameReadCallbackConfiguration(false)
}
