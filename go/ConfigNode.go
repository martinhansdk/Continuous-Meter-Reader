package main

import (
	"MeterReader"
	"flag"
	"github.com/tarm/serial"
	"log"
)

func main() {
	var serialport = flag.String("serial", "/dev/ttyUSB0", "The serial port to use to connect")
	var id = flag.Int("id", -1, "Set the meter id")

	flag.Parse()

	config := &serial.Config{Name: *serialport, Baud: 9600}

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

	changed := false
	if *id != -1 {
		changed = true
		var _id = int32(*id)
		settings.MeterId = &_id
	}

	if changed {
		MeterReader.PrintSettings(settings)
		MeterReader.SendSettings(ser, settings)

		MeterReader.PrintSettings(<-settingschan)
	}
}
