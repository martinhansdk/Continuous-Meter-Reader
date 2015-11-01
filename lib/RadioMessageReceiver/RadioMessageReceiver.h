#ifndef RADIOMESSAGE_RECEIVER_H
#define RADIOMESSAGE_RECEIVER_H

#include <RF24.h>

#ifdef __AVR__
// avr does not have endian.h and is already little endian
#define htole64(val) val
#define le64toh(val) val
#else
#include <endian.h>
#endif

typedef uint64_t header_t;

struct MessageHeader {
  uint32_t senderId;
  uint16_t sequence;
  uint32_t messageNumber;
  uint8_t chunkNumber;
  uint16_t bytes;
  bool complete;

  bool isSamePackage(const MessageHeader &newHeader) const {
    return (senderId == newHeader.senderId) &&
           (sequence == newHeader.sequence) &&
           (messageNumber == newHeader.messageNumber) &&
           (chunkNumber == newHeader.chunkNumber);
  }

  bool isNextPackage(const MessageHeader &newHeader) const {
    return (senderId == newHeader.senderId) &&
           (sequence == newHeader.sequence) &&
           (messageNumber == newHeader.messageNumber) &&
           (chunkNumber + 1 == newHeader.chunkNumber);
  }

  bool isNewMessage(const MessageHeader &newHeader) const {
    return (senderId == newHeader.senderId) &&
           (
             (sequence != newHeader.sequence) ||
             (messageNumber != newHeader.messageNumber)
           ) &&
           (newHeader.chunkNumber == 0);
  }

};

template<int MAX_MESSAGE_LEN>
struct NodeReceiver {
  bool empty;
  MessageHeader message;
  uint8_t data[MAX_MESSAGE_LEN];


  NodeReceiver() : empty(true) {}

  bool isRightReceiver(const MessageHeader &newHeader) const {
    return empty || (message.senderId == newHeader.senderId);
  }

  bool process(const MessageHeader &newHeader, const uint8_t *newData) {

    if( !empty && message.isSamePackage(newHeader) ) {
      // we already got this message once. Probably the ACK did not make it back to the sender.
    } else if( !empty && message.isNextPackage(newHeader) ) {
      // this is the next package in the sequence, we've been expecting it
      if(message.bytes + newHeader.bytes > MAX_MESSAGE_LEN) {
        // message too long - discard entire message
        message.bytes = 0;
      } else {
        memcpy(data + message.bytes, newData, newHeader.bytes);
        message.bytes += newHeader.bytes;
        message.chunkNumber = newHeader.chunkNumber;
        message.complete = newHeader.complete;
      }


    } else if( empty || message.isNewMessage(newHeader) ) {
      // this is a new message
      memcpy(data, newData, newHeader.bytes);
      message = newHeader;
      empty = false;

    } else {
      // huh? Something is wrong. This package is older than the last one we got or is part
      // of a previously discarded message. Let's throw it away
    }

    return message.complete;
  }

};


template<int MAX_NODES, int MAX_MESSAGE_LEN>
class RadioMessageReceiver {
    RF24& radio;

    NodeReceiver<MAX_MESSAGE_LEN> nodes[MAX_NODES];


    uint8_t buffer[32];

  public:
    RadioMessageReceiver(RF24 &radio) : radio(radio) {
    }

    /** Call this method periodically to process incoming radio packets.
    It assembles messages from multiple receivers from multiple packets.

    Returns true when a complete message has been received. Then call read()
    to access the received message.
    */

    bool available() {
      if(radio.available()) {
        int bytes = radio.getDynamicPayloadSize();
        if(bytes < 1) {
          // Corrupt payload has been flushed
          return false;
        }
        radio.read(&buffer, sizeof(buffer));

        MessageHeader header;
        const uint8_t *payload;

        bool headerOk = decodeHeader(buffer, payload, header, bytes);

        if(headerOk) {
          int i = findNodeSlotForNewHeader(header);
          if(i == -1) {
            // sorry, can't handle that many different nodes. Discarding message.
            return false;
          }

          return nodes[i].process(header, payload);
        }
      }

      return false;
    }

    /** Return the received message in the form of

    buf :  a pointer to the data
    header: a pointer to the header

    return value: number of payload bytes

    Only call this when available() has returned true. The data must be processed
    before the next call to available(). available() will invalidate the pointers.
    If the data is needed longer, it must be copied.

    */
    uint16_t read(uint8_t *&buf, const MessageHeader *&header) {
      int i = findCompleteMessageSlot();
      if(i == -1) {
        return 0;
      }

      buf = &(nodes[i].data[0]);
      header = &(nodes[i].message);
      nodes[i].message.complete = false; // make sure this message is only returned once
      return nodes[i].message.bytes;
    }

    RF24& getRadio() const {
      return radio;
    }

  private:
    bool decodeHeader(const uint8_t *buf, const uint8_t *&payload, MessageHeader &header, uint8_t bytes) {
      if(bytes < sizeof(header_t)) {
        return false;
      }

      header_t rawHeader = le64toh(*((header_t*)buf));

      header.senderId      = (rawHeader >> 57) & 0x7F;
      header.sequence      = (rawHeader >> 41) & 0xFFFF;
      header.messageNumber = (rawHeader >>  9) & 0xFFFFFFFF;
      header.chunkNumber   = (rawHeader >>  1) & 0xFF;
      header.complete      =  rawHeader        & 0x1;
      header.bytes         = bytes - sizeof(header_t);

      payload = buf + sizeof(header_t);

      return true;
    }


    int findNodeSlotForNewHeader(const MessageHeader &header) const {
      for(int i = 0 ; i < MAX_NODES ; i++) {
        if(nodes[i].isRightReceiver(header)) {
          return i;
        }
      }
      return -1;
    }

    int findCompleteMessageSlot() const {
      for(int i = 0 ; i < MAX_NODES ; i++) {
        if(!nodes[i].empty && nodes[i].message.complete) {
          return i;
        }
      }
      return -1;
    }
};

#endif