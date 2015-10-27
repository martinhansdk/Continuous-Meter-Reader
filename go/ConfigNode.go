package main

import (
	"MeterReader"
	"flag"
	"fmt"
	"github.com/tarm/serial"
	"log"
	"math"
	"strconv"
	"time"
)

type edgeAmounts struct {
	changed bool
	sensors int
	amounts []uint32
}

func (i *edgeAmounts) String() string {
	return fmt.Sprintf("%d", (*i).amounts)
}

// The second method is Set(value string) error
func (i *edgeAmounts) Set(value string) error {
	tmp, err := strconv.ParseUint(value, 10, 32)
	if err != nil {
		return err
	} else {
		(*i).amounts = append((*i).amounts, uint32(tmp))
	}

	i.sensors++
	i.changed = true
	return nil
}

func main() {
	var serialport = flag.String("serial", "/dev/ttyUSB0", "The serial port to use to connect")
	var id = flag.Uint("id", math.MaxUint32, "Set the meter id")
	var seriesId = flag.Uint("series", 0, "Set the series id")
	var calibrate = flag.Bool("calibrate", false, "Start calibration")
	var analog = flag.Bool("analog", false, "Set sampling mode to analog")
	var serialProtocol = flag.Bool("serialproto", false, "Use serial communication to transfer updates")
	var wirelessProtocol = flag.Bool("wirelessproto", false, "Use wireless to transfer updates")
	var digital = flag.Bool("digital", false, "Set sampling mode to digital")
	var risingEdgeAmounts edgeAmounts
	var fallingEdgeAmounts edgeAmounts
	flag.Var(&risingEdgeAmounts, "rising", "Rising edge amount")
	flag.Var(&fallingEdgeAmounts, "falling", "Falling edge amount")

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

	if *id != math.MaxUint32 {
		changedSettings = true
		var _id = uint32(*id)
		settings.MeterId = &_id
	}

	if *seriesId != 0 {
		changedSettings = true
		var _id = uint32(*seriesId)
		settings.SeriesId = &_id
	}

	if fallingEdgeAmounts.changed || risingEdgeAmounts.changed {
		if fallingEdgeAmounts.sensors != risingEdgeAmounts.sensors {
			log.Fatal("Number of rising and falling edge amounts must be the same.")
		}

		if fallingEdgeAmounts.sensors > 6 {
			log.Fatal("Maximum 6 edges allowed.")
		}

		settings.FallingEdgeAmounts = fallingEdgeAmounts.amounts
		settings.RisingEdgeAmounts = risingEdgeAmounts.amounts

		changedSettings = true
	}

	if *digital || *analog {
		if *digital && *analog {
			log.Fatal("Can't set to both digital and analog sampling mode.")
		}

		var mode MeterReader.Settings_SamplingMode
		if *digital {
			mode = MeterReader.Settings_DIGITAL
		} else {
			mode = MeterReader.Settings_ANALOG
		}
		settings.SamplingMode = &mode
		changedSettings = true
	}

	if *serialProtocol || *wirelessProtocol {
		if *serialProtocol && *wirelessProtocol {
			log.Fatal("Can't use both serial and wireless communication protocol.")
		}

		var mode MeterReader.Settings_CommunicationChannel
		if *serialProtocol {
			mode = MeterReader.Settings_SERIAL
		} else {
			mode = MeterReader.Settings_WIRELESS
		}
		settings.CommunicationChannel = &mode
		changedSettings = true
	}

	if changedSettings {
		MeterReader.PrintSettings(settings)
		MeterReader.SendSettings(ser, settings)
		expectSettingsResponse = true
	}

	if expectSettingsResponse {
		MeterReader.PrintSettings(<-settingschan)
	}
}
