#include "CommunicateSerial.h"
#include "RadioMessageReceiver.h"
#include "MeterReader_pb.h"
#include "RF24.h"
#include "pb_decode.h"
#include "crc32.h"

RF24 radio(9, 10);
RadioMessageReceiver<4, MeterReader_Message_size> receiver(radio);
//const uint64_t radioAddress = 286093099LL;

const char* addresses[] = {"meterS", "meterR"};

void setup() {
  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
  radio.openWritingPipe((const uint8_t*)addresses[0]);
  radio.openReadingPipe(1, (const uint8_t*)addresses[1]);
  radio.enableDynamicPayloads();

  Serial.begin(57600);

  // the serial port does not work reliably right after initializing
  // the port, this delay makes sure it's ready before we use it
  delay(500);
  sendLog(Serial, MeterReader_LogMessage_Type_NOTE, "Rebooted");

  // Start the radio listening for data
  radio.startListening();
}

void loop() {
  /*
  unsigned long got_time;
  if( radio.available()){
                                                                  // Variable for the received timestamp
    while (radio.available()) {                                   // While there is data ready
      Serial.println("Got some!");
      radio.read( &got_time, sizeof(unsigned long) );             // Get the payload
    }
  }
  return;
  */
  if(receiver.available()) {
    uint8_t *data;

    const MessageHeader *header;
    uint8_t len = receiver.read(data, header);

    if(len == 0) {
      return;
    }
    MeterReader_Message message;

    pb_istream_t stream = pb_istream_from_buffer(data, len);

    // Now we are ready to decode the message.
    bool ok = pb_decode(&stream, MeterReader_Message_fields, &message);

    if(ok) {
      sendMessage(Serial, message);
      if(message.which_message == MeterReader_Message_update_tag) {
        if(header->senderId == message.message.update.meterId && header->sequence == message.message.update.seriesId) {
        }
      }
    }


  }

}