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

func HandleProtoClient(conn io.ReadCloser, c chan *CounterUpdate) {
	var len uint32

	log.Println("Connection established")
	//Close the connection when the function exits
	defer conn.Close()

	for {

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
				seen = 0
				garbage += 1
			}

		}
		if garbage > 0 {
			log.Printf("Discarded %d bytes of garbage\n", garbage)
		}

		//Read the length field
		err := binary.Read(conn, binary.BigEndian, &len)
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

		fp, _ := os.Create("/tmp/message.pdata")
		fp.Write(data)
		fp.Close()

		// Read the checksum and match it against the received data
		crc := crc32.New(crcTable)
		crc.Write(data)
		var expectedCRC uint32
		err = binary.Read(conn, binary.BigEndian, &expectedCRC)
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
			c <- counterUpdate
		}

		settings := protodata.GetSettings()
		if settings != nil {
			b, err := json.Marshal(settings)
			if err != nil {
				log.Println("JSON encode: Error %s", err)
			} else {
				os.Stdout.Write(b)
			}
		}
	}
}
