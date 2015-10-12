
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
#include <iostream>

static int HEADER_SIZE = sizeof(uint64_t);

uint64_t makePackageId(uint32_t id, uint16_t sequence, uint32_t messageNumber, uint8_t chunkNumber, bool last) {
	uint64_t packageId = (uint64_t)(id & 0x7F) << 57
	                   | (uint64_t)(sequence & 0xFFFF) << 41
	                   | (uint64_t)(messageNumber & 0xFFFFFFFF) << 9
	                   | (uint64_t)(chunkNumber & 0xFF) << 1
	                   | (uint64_t)last;

	return htole64(packageId);
}

class RadioMessageSender {
	uint8_t buffer[32];
	uint8_t bytesInBuffer;
	uint32_t id;
	uint16_t sequence;
	uint32_t messageNumber;
	uint8_t chunkNumber;
	RF24& radio;

public:
    RadioMessageSender(uint32_t myId, uint16_t sequence, RF24 &radio) : id(myId), sequence(sequence), messageNumber(0), radio(radio) {};
    void startMessage();
    bool write(const uint8_t *data, uint8_t len, bool lastWriteInMessage);

private:
	void writeHeader(bool last);
};

void RadioMessageSender::startMessage() {
	messageNumber++;
	chunkNumber = 0;
	bytesInBuffer = HEADER_SIZE;
}


void RadioMessageSender::writeHeader(bool last) {
	uint64_t thisSequenceId = makePackageId(id, sequence, messageNumber, chunkNumber, last);
	memcpy(buffer, &thisSequenceId, HEADER_SIZE); 
	chunkNumber++;
}

bool RadioMessageSender::write(const uint8_t *data, uint8_t len, bool lastWriteInMessage) {
	while(len > 0) {
		size_t maxLen = min((unsigned long)len, sizeof(buffer) - bytesInBuffer);
		memcpy(&buffer[bytesInBuffer], data, maxLen);
		bytesInBuffer += maxLen;
		len -= maxLen;
		data += maxLen;

		if(bytesInBuffer == sizeof(buffer) || len == 0) {
			bool lastChunk = lastWriteInMessage && len == 0;
			writeHeader(lastChunk);
			bool ok = radio.write(buffer, bytesInBuffer);
			if(!ok)
				return false;

			bytesInBuffer = HEADER_SIZE;
		}
	}
	return true;
}

#endif