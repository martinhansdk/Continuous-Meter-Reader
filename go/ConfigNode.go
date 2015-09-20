package main

import (
	"MeterReader"
	"flag"
	"github.com/tarm/serial"
	"log"
	"time"
)

func main() {
	var serialport = flag.String("serial", "/dev/ttyUSB0", "The serial port to use to connect")
	var id = flag.Int("id", -1, "Set the meter id")
	var calibrate = flag.Bool("calibrate", false, "Start calibration")

	flag.Parse()

	config := &serial.Config{Name: *serialport, Baud: 57600}
	// Opening the port resets the device. First the bootloader runs for half a second, then the actual application loads.
	// Wait for this to happen before attempting to talk to the device.
	time.Sleep(1 * time.Second)

	updatechan := make(chan *MeterReader.CounterUpdate)
	settingschan := make(chan *MeterReader.Settings)

	go func() {
		// drain the channel for updates which we won't need
		for range updatechan {
		}
	}()

	ser, err := serial.OpenPort(config)

	if err != nil {
		log.Fatal(err)
	}

	go MeterReader.HandleProtoClient(ser, updatechan, settingschan)

	log.Print("Waiting for current settings")
	settings := <-settingschan
	log.Print("Got current settings")

	MeterReader.PrintSettings(settings)

	log.Print("id=", *id)

	changedSettings := false
	expectSettingsResponse := false

	if *calibrate {
		expectSettingsResponse = true
		MeterReader.SendStartCalibration(ser)
	}

	if *id != -1 {
		changedSettings = true
		expectSettingsResponse = true
		var _id = int32(*id)
		settings.MeterId = &_id
	}

	if changedSettings {
		MeterReader.PrintSettings(settings)
		MeterReader.SendSettings(ser, settings)
	}

	if expectSettingsResponse {
		MeterReader.PrintSettings(<-settingschan)
	}
}
