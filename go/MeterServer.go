package main

import (
	"MeterReader"
	"github.com/googollee/go-socket.io"
	"github.com/tarm/serial"
	"log"
	"net"
	"net/http"
	"os"
	"os/signal"
	"path/filepath"
	"syscall"
	"time"
)

func main() {
	log.Println("Started Meter Receiver Server")

	//Listen to the TCP port for connections from meter readers
	updatechan := make(chan *MeterReader.CounterUpdate)
	settingschan := make(chan *MeterReader.Settings)

	go func() {
		// drain the channel for settings which we won't need
		for range settingschan {
		}
	}()

	log.Println("Listening to port 2110 for meter events")
	listener, err := net.Listen("tcp", "127.0.0.1:2110")
	if err != nil {
		log.Fatal(err)
	}

	go func() {
		for {
			if conn, err := listener.Accept(); err == nil {
				//If err is nil then that means that data is available for us so we take up this data and pass it to a new goroutine
				go MeterReader.HandleProtoClient(conn, updatechan, settingschan)
			} else {
				continue
			}
		}
	}()

	// Listen to the serial port
	go func() {

		for {

			serialPorts, err := filepath.Glob("/dev/ttyUSB*")

			if err == nil {
				for i := range serialPorts {
					config := &serial.Config{Name: serialPorts[i], Baud: 57600}
					ser, err := serial.OpenPort(config)

					if err != nil {
						// ok, try the next port
					} else {
						log.Printf("Opened port %s", serialPorts[i])
						MeterReader.HandleProtoClient(ser, updatechan, settingschan)
					}
				}
			}

			time.Sleep(time.Second) // try to open again in a second
		}
	}()

	// translate the events from the meters to meter state messages
	msh := MeterReader.NewMeterStateHandler()
	tch := msh.Handle(updatechan)

	// socket.io server
	log.Println("Starting socket.io server")
	server, err := socketio.NewServer(nil)
	if err != nil {
		log.Fatal(err)
	}
	server.On("connection", func(so socketio.Socket) {
		log.Println("on connection")
		so.Join("updates")
		so.On("disconnection", func() {
			log.Println("on disconnect")
		})
	})
	server.On("error", func(so socketio.Socket, err error) {
		log.Println("error:", err)
	})
	go func() {
		for msg := range tch {
			log.Println("JSON to UI:", msg)
			server.BroadcastTo("updates", "meter update", msg)
		}
	}()

	// serve the user
	log.Println("Preparing json api")
	api := MeterReader.MakeAPI()

	http.Handle("/api/", http.StripPrefix("/api", api.MakeHandler()))
	http.Handle("/socket.io/", server)
	http.Handle("/static/", http.StripPrefix("/static", http.FileServer(http.Dir("src/MeterReader/static"))))

	go func() {
		log.Println("Listening on port 2111 for http traffic")
		log.Fatal(http.ListenAndServe(":2111", nil))
	}()

	// stop on SIGINT and SIGTERM
	ch := make(chan os.Signal)
	signal.Notify(ch, syscall.SIGINT, syscall.SIGTERM)
	log.Println(<-ch)
}
