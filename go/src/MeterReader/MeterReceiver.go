package MeterReader

import (
	"encoding/binary"
	"encoding/json"
	"github.com/golang/protobuf/proto"
	"hash/crc32"
	"io"
	"log"
	"os"
)

const MAX_PROTOBUF_MSG_LEN = 121

var crcTable = crc32.MakeTable(crc32.IEEE) // see http://golang.org/pkg/hash/crc32/#pkg-constants

func HandleProtoClient(conn io.ReadCloser, updateChan chan *CounterUpdate, settingsChan chan *Settings) {
	var len uint32

	log.Println("Connection established")
	//Close the connection when the function exits
	defer conn.Close()

	for {

		log.Print("Waiting for data")

		// Find the first occurrence of the magic string which is AA
		buf := make([]byte, 1)
		seen := 0
		garbage := 0
		for seen != 2 {
			_, err := conn.Read(buf)
			if err == io.EOF {
				log.Printf("Preamble: EOF reached\n")
				break
			} else if err != nil {
				log.Printf("Preamble: Error %s\n", err)
				break
			}

			if buf[0] == 'A' {
				seen += 1
			} else {
				if seen > 0 {
					log.Printf("Discarded: %d", int('A'))
				}
				log.Printf("Discarded: %d", int(buf[0]))
				seen = 0
				garbage += 1
			}

		}
		if garbage > 0 {
			log.Printf("Discarded %d bytes of garbage\n", garbage)
		}

		//Read the length field
		err := binary.Read(conn, binary.LittleEndian, &len)
		if err == io.EOF {
			log.Printf("Length: EOF reached\n")
			break
		} else if err != nil {
			log.Printf("Length: Error %s\n", err)
			break
		}

		log.Println("len=", len)

		if len > MAX_PROTOBUF_MSG_LEN {
			log.Println("Message length unrealistically large. Skipping. len=%s", len)
			continue
		}

		//Create a data buffer of type byte slice with capacity for the message
		data := make([]byte, len)
		//Read the data waiting on the connection and put it in the data buffer
		n, err := io.ReadFull(conn, data)
		if err == io.EOF {
			log.Printf("Message: EOF reached\n")
			break
		} else if err != nil {
			log.Printf("Message: Error %s\n", err)
			break
		}
		log.Printf("n=%d", n)

		fp, _ := os.Create("/tmp/message.pdata")
		fp.Write(data)
		fp.Close()

		// Read the checksum and match it against the received data
		crc := crc32.New(crcTable)
		crc.Write(data)
		var expectedCRC uint32
		err = binary.Read(conn, binary.LittleEndian, &expectedCRC)
		if err == io.EOF {
			log.Printf("Checksum: EOF reached\n")
			break
		} else if err != nil {
			log.Printf("Checksum: Error %s\n", err)
			break
		}

		if crc.Sum32() != expectedCRC {
			log.Printf("Checksum mismatch, skipping. Header says 0x%08x, calculated 0x%08x\n", expectedCRC, crc.Sum32())
			continue
		}

		protodata := new(Message)
		//Convert all the data retrieved into the Message struct type
		err = proto.Unmarshal(data[0:n], protodata)
		if err != nil {
			log.Printf("Unmarshal: Error %s\n", err)
			break
		}

		counterUpdate := protodata.GetUpdate()
		if counterUpdate != nil {
			updateChan <- counterUpdate
		}

		settings := protodata.GetSettings()
		if settings != nil {
			settingsChan <- settings
		}

		logmsg := protodata.GetLog()
		if logmsg != nil {
			log.Printf("%s : Received '%s'", logmsg.GetType().String(), logmsg.GetText())
		}
	}
}

func SendSettings(conn io.Writer, settings *Settings) {
	msg := new(Message)
	msg.Settings = settings

	sendMessage(conn, msg)
}

func SendStartCalibration(conn io.Writer) {
	msg := new(Message)
	msg.Calibrate = new(StartCalibration)

	sendMessage(conn, msg)
}

func sendMessage(conn io.Writer, message *Message) {
	_, err := io.WriteString(conn, "AA") // preamble
	if err != nil {
		log.Fatal("unable to send data: ", err)
	}

	data, err := proto.Marshal(message)
	if err != nil {
		log.Fatal("marshaling error: ", err)
	}

	err = binary.Write(conn, binary.LittleEndian, int32(len(data)))
	if err != nil {
		log.Fatal("unable to send data: ", err)
	}

	_, err = conn.Write(data)
	if err != nil {
		log.Fatal("unable to send data: ", err)
	}

	crc := crc32.New(crcTable)
	crc.Write(data)
	err = binary.Write(conn, binary.LittleEndian, int32(crc.Sum32()))
	if err != nil {
		log.Fatal("unable to send data: ", err)
	}
}

func PrintSettings(settings *Settings) {
	b, err := json.Marshal(settings)
	if err != nil {
		log.Println("JSON encode: Error %s", err)
	} else {
		os.Stdout.Write(b)
		io.WriteString(os.Stdout, "\n")
	}
}
