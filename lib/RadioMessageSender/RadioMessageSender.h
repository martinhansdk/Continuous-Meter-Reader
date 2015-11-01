
/** Sends a message over a RF24 link.
The message may be larger than the maximum payload size of 32 bytes and will then be split into several chunks.

Every chunk is prepended with a header containing

  the id of this node (7 bits)
  the current sequence of messages (should change on every reboot) (16 bits)
  the message number since reboot (32 bits)
  the chunk number within this message (8 bits)
  a bit indicating if this is the last message (1 bit)

This makes it possible for the receiver to determine

  which node the message is from
  if the message has been received before (retries with lost acks)
  if a message is missing in the sequence
  when the message is complete
*/

#ifndef RADIOMESSAGE_SENDER_H
#define RADIOMESSAGE_SENDER_H

#include "RF24.h"
#include <stdint.h>

static int RADIO_HEADER_SIZE = sizeof(uint64_t);

class RadioMessageSender {
    uint8_t buffer[32];
    uint8_t bytesInBuffer;
    uint32_t id;
    uint16_t sequence;
    uint32_t messageNumber;
    uint8_t chunkNumber;
    RF24& radio;

  public:
    RadioMessageSender(RF24 &radio) : radio(radio) {};
    void begin(uint32_t myId, uint16_t _sequence);
    void startMessage();
    bool write(const uint8_t *data, uint8_t len, bool lastWriteInMessage);

  private:
    void writeHeader(bool last);
};


#endif