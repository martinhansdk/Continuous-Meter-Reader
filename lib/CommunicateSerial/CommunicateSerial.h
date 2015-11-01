#ifndef _COMMUNICATE_SERIAL_H
#define _COMMUNICATE_SERIAL_H

#include <Stream.h>
#include <MeterReader_pb.h>
#include <crc32.h>
#include "pb_decode.h"

#ifdef __AVR__
// avr does not have endian.h and is already little endian
#define htole32(val) val
#define le32toh(val) val
#else
#include <endian.h>
#endif

void sendMessage(Stream &s, MeterReader_Message &message);
void sendCounterUpdate(Stream &s, int32_t meterId, uint32_t seriesId, uint64_t currentValue);
void sendSettings(Stream &s, MeterReader_Settings &settings);
void sendLog(Stream &s, MeterReader_LogMessage_Type type, const char* text);
bool read_callback(pb_istream_t *stream, uint8_t *buf, size_t count);

class StreamState {
  public:
    uint32_t crc;
    Stream &stream;

    StreamState(Stream &s) : crc(~0L), stream(s) {}

    uint32_t getCRC() {
      return ~crc;
    }

    void crc_add(const uint8_t* buf, int count) {
      while (count--) {
        crc = crc_update(crc, *buf++);
      }
    }
};

template <typename M, const pb_field_t* A>
class Receiver {
    typedef enum _State {
      READ_PREAMBLE,
      READ_LEN,
      READ_MSG,
      READ_CHECKSUM
    } State;
    State state;
    Stream &s;
    int preamble_seen;
    size_t len;
    uint32_t calculated_crc;
  public:
    M message;
    Receiver(Stream &s) : state(READ_PREAMBLE), s(s) {
      preamble_seen = 0;
    }

    bool process() {
      int available = s.available();
      bool ready = false;
      if(available) {
        switch(state) {
        case READ_PREAMBLE:
          // the preamble is the string 'AA'.
          // Everything before that is regarded as garbage and discarded
          if(s.read() == 'A') {
            preamble_seen += 1;
          } else {
            preamble_seen = 0;
          }

          if(preamble_seen == 2) {
            preamble_seen = 0;
            state = READ_LEN;
          }
          break;
        case READ_LEN:
          if(available >= 4) {
            s.readBytes((char*)&len, 4);
            len = le32toh(len);
            if(len > MeterReader_Message_size) {
              sendLog(s, MeterReader_LogMessage_Type_ERROR, "Too big");
              state = READ_PREAMBLE; // too big, look for the next one
            } else {
              state = READ_MSG;
            }
          }
          break;
        case READ_MSG: {
          StreamState sstate(s);
          pb_istream_t instream = {&read_callback, (void*)&sstate, len, ""};

          bool ok = pb_decode(&instream, A, &message);


          if(!ok) {
            sendLog(s, MeterReader_LogMessage_Type_ERROR, "Decode error");
            state = READ_PREAMBLE; // error, start over
          } else {
            calculated_crc = sstate.getCRC();
            state = READ_CHECKSUM;
          }
        }
        break;
        case READ_CHECKSUM:
          if(available >= 4) {
            uint32_t crc;
            s.readBytes((char*)&crc, 4);
            crc = le32toh(crc);

            state = READ_PREAMBLE; // done
            if(crc == calculated_crc) {
              ready = true;
            } else {
              // sendLog(s, MeterReader_LogMessage_Type_ERROR, "CRC mismatch");
            }
          }
          break;
        default:
          state = READ_PREAMBLE;
        }
      }
      return ready;
    }
};

#endif