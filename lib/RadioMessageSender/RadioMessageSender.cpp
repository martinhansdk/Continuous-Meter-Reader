#include "RadioMessageSender.h"

#ifdef __AVR__
// avr does not have endian.h and is already little endian
#define htole64(val) val
#define le64toh(val) val
#else
#include <endian.h>
#endif

uint64_t makePackageId(uint32_t id, uint16_t sequence, uint32_t messageNumber, uint8_t chunkNumber, bool last) {
  uint64_t packageId = (uint64_t)(id & 0x7F) << 57
                       | (uint64_t)(sequence & 0xFFFF) << 41
                       | (uint64_t)(messageNumber & 0xFFFFFFFF) << 9
                       | (uint64_t)(chunkNumber & 0xFF) << 1
                       | (uint64_t)last;

  return htole64(packageId);
}

void RadioMessageSender::begin(uint32_t myId, uint16_t _sequence) {
  id = myId;
  sequence = _sequence;
}

void RadioMessageSender::startMessage() {
  messageNumber++;
  chunkNumber = 0;
  bytesInBuffer = RADIO_HEADER_SIZE;
}


void RadioMessageSender::writeHeader(bool last) {
  uint64_t thisSequenceId = makePackageId(id, sequence, messageNumber, chunkNumber, last);
  memcpy(buffer, &thisSequenceId, RADIO_HEADER_SIZE);
  chunkNumber++;
}

bool RadioMessageSender::write(const uint8_t *data, uint8_t len, bool lastWriteInMessage) {
  while(len > 0 || lastWriteInMessage) {
    size_t maxLen = min((unsigned long)len, sizeof(buffer) - bytesInBuffer);
    memcpy(&buffer[bytesInBuffer], data, maxLen);
    bytesInBuffer += maxLen;
    len -= maxLen;
    data += maxLen;

    if(bytesInBuffer == sizeof(buffer) || (len == 0 && lastWriteInMessage)) {
      bool lastChunk = lastWriteInMessage && len == 0;
      writeHeader(lastChunk);
      bool ok = radio.write(buffer, bytesInBuffer);
      if(!ok)
        return false;

      bytesInBuffer = RADIO_HEADER_SIZE;

      if(lastWriteInMessage && len == 0)
        break;
    }
  }
  return true;
}