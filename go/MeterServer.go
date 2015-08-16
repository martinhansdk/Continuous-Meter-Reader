package main

import (
	"MeterReader"
	"fmt"
	"log"
	"net"
	"net/http"
)

func main() {
	fmt.Println("Started Meter Receiver Server")

	// serve the user
	api := MeterReader.MakeAPI()

	http.Handle("/api/", http.StripPrefix("/api", api.MakeHandler()))

	http.Handle("/static/", http.StripPrefix("/static", http.FileServer(http.Dir("src/MeterReader/static"))))

	go func() {
		log.Fatal(http.ListenAndServe(":2111", nil))
	}()

	// receive new updates from the meters
	c := make(chan *MeterReader.CounterUpdate)
	go func() {
		mdb := MeterReader.NewMeterDB()
		meters := mdb.GetMeterState()
		for {
			message := <-c
			MeterReader.WriteValuesToDatabase(mdb, meters, message)
		}
	}()

	//Listen to the TCP port for connections from meter readers
	listener, err := net.Listen("tcp", "127.0.0.1:2110")
	MeterReader.CheckError(err)
	for {
		if conn, err := listener.Accept(); err == nil {
			//If err is nil then that means that data is available for us so we take up this data and pass it to a new goroutine
			go MeterReader.HandleProtoClient(conn, c)
		} else {
			continue
		}
	}
}
