package MeterReader

import (
	"encoding/binary"
	"fmt"
	"github.com/golang/protobuf/proto"
	"hash/crc32"
	"io"
	"net"
	"os"
)

const MAX_PROTOBUF_MSG_LEN = 16

var castagnoliTable = crc32.MakeTable(crc32.IEEE) // see http://golang.org/pkg/hash/crc32/#pkg-constants

func HandleProtoClient(conn net.Conn, c chan *CounterUpdate) {
	var len uint32

	fmt.Println("Connection established")
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
				break
			}

			CheckError(err)

			if buf[0] == 'A' {
				seen += 1
			} else {
				seen = 0
				garbage += 1
			}

		}
		if garbage > 0 {
			fmt.Printf("Discarded %d bytes of garbage\n", garbage)
		}

		//Read the length field
		err := binary.Read(conn, binary.BigEndian, &len)
		if err == io.EOF {
			break
		}

		CheckError(err)
		fmt.Println("len=", len)

		if len > MAX_PROTOBUF_MSG_LEN {
			fmt.Println("Message length unrealistically large. Skipping. len=", len)
			continue
		}

		//Create a data buffer of type byte slice with capacity for the message
		data := make([]byte, len)
		//Read the data waiting on the connection and put it in the data buffer
		n, err := conn.Read(data)
		if err == io.EOF {
			break
		}
		CheckError(err)

		// Read the checksum and match it against the received data
		crc := crc32.New(castagnoliTable)
		crc.Write(data)
		var expectedCRC uint32
		err = binary.Read(conn, binary.BigEndian, &expectedCRC)
		if err == io.EOF {
			break
		}
		CheckError(err)

		if crc.Sum32() != expectedCRC {
			fmt.Println("Checksum mismatch, skipping")
			continue
		}

		//Create an struct pointer of type ProtobufTest.TestMessage struct
		protodata := new(CounterUpdate)
		//Convert all the data retrieved into the ProtobufTest.TestMessage struct type
		err = proto.Unmarshal(data[0:n], protodata)
		CheckError(err)
		//Push the protobuf message into a channel
		c <- protodata
	}
}

type Meter struct {
	MeterId       int32
	Unit          string
	CurrentSeries uint32
	StartCount    uint64
	LastCount     uint64
}

func WriteValuesToDatabase(mdb *MeterDB, meters map[int32]*Meter, msg *CounterUpdate) {

	//Retreive client information from the protobuf message
	MeterId := msg.GetMeterId()

	meter, ok := meters[MeterId]
	if !ok {
		meter = &Meter{MeterId: MeterId}
		meters[MeterId] = meter
	}

	SeriesId := msg.GetSeriesId()
	CurrentCounterValue := msg.GetCurrentCounterValue()

	if meter.CurrentSeries != SeriesId {
		meter.CurrentSeries = SeriesId
		meter.StartCount = meter.LastCount
	}
	meter.LastCount = meter.StartCount + CurrentCounterValue

	fmt.Printf("meterid=%d series=%d counter=%d -> absolute=%d\n", MeterId, SeriesId, CurrentCounterValue, meter.LastCount)

	//mdb.InsertMeasurement(MeterId, meter.LastCount)
}

func CheckError(err error) {
	if err != nil {
		fmt.Printf("Fatal error: %s", err.Error())
		os.Exit(1)
	}
}
