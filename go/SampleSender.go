package main

import (
	"MeterReader"
	"io"
	"log"
	"math/rand"
	"net"
	"time"
)

const host string = "localhost:2110"

// Setup
var NUM_METERS              int = 4
var UPDATE_INTERVAL_SECONDS int = 5
var NUM_GARBAGE_BYTES       int = 49

//----------------------------------------------------------------------
func meterId() *uint32 {
  var x uint32 = uint32(1+rand.Intn(NUM_METERS))
  return &x
}

func send_garbage(sock io.Writer, amount int) {
	var garbage []byte
	garbage = make([]byte, amount, amount)

	for i := 0; i < amount; i++ {
		garbage[i] = byte(rand.Intn(255))
	}

	sock.Write(garbage)
}

func main() {
	s, err := net.Dial("tcp", host)
	if err != nil {
		log.Fatalf("ERROR: Unable to connect to %s", host)
	}

	var series uint32
	var count uint64
	for {
		update := new(MeterReader.CounterUpdate)
		update.MeterId = meterId()
		if rand.Intn(10) == 1 {
			// new series
			series += 1
			count = 0
		}

		count += uint64(rand.Int63n(10))

		update.SeriesId = &series
		update.CurrentCounterValue = &count

		MeterReader.SendUpdate(s, update)
		if rand.Intn(10) < 3 {
			send_garbage(s, rand.Intn(NUM_GARBAGE_BYTES)+1)
		}

		time.Sleep(time.Duration(rand.Intn(UPDATE_INTERVAL_SECONDS)) * time.Second)
	}
}
