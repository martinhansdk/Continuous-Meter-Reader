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

var meterId uint32 = 1

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

		update.MeterId = &meterId
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
			send_garbage(s, rand.Intn(49)+1)
		}

		time.Sleep(1 * time.Second)
	}
}
