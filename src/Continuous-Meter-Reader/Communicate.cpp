#include "Communicate.h"
#include "crc32.h"
#include "pb_encode.h"

void write_uint32(Stream &s, uint32_t val) {
  uint8_t bytes[4];
  bytes[0] = (val >> 24) & 0xFF;
  bytes[1] = (val >> 16) & 0xFF;
  bytes[2] = (val >> 8) & 0xFF;
  bytes[3] = val & 0xFF;
  
  s.write(bytes, 4);
}

void sendError(Stream &s, const char* errmsg) {
  s.write(errmsg);
}

class OutputStreamState {
public:
  uint32_t crc;
  Stream &stream;

  OutputStreamState(Stream &s) : crc(~0L), stream(s) {}

  uint32_t getCRC() {
    return ~crc;
  }
};

bool write_callback(pb_ostream_t *stream, const uint8_t *buf, size_t count)
{
  OutputStreamState *state = (OutputStreamState*) stream->state;

  size_t sent = state->stream.write(buf, count);
  bool ok = (sent == count);

  while (count--) {
    state->crc = crc_update(state->crc, *buf++);
  }

  return ok;
}

void sendMessage(Stream &s, MeterReader_Message &message) {
  // get the message size
  pb_ostream_t sizestream = {0, 0, SIZE_MAX, 0, ""};
  bool ok = pb_encode(&sizestream, MeterReader_Message_fields, &message);
  if(!ok) {
    sendError(s, sizestream.errmsg);
    return;
  }


  // send the actual message content
  s.write("AA"); // preamble
  write_uint32(s, sizestream.bytes_written); // length field
  OutputStreamState state(s);
  pb_ostream_t stream = {&write_callback, &state, SIZE_MAX, 0, ""};
  ok = pb_encode(&stream, MeterReader_Message_fields, &message); // send data
  if(ok) {  
    write_uint32(s, state.getCRC());
  } else {
    sendError(s, stream.errmsg);
  }

}

void sendCounterUpdate(Stream &s, int32_t meterId, uint32_t seriesId, uint64_t currentValue) {
	MeterReader_Message message;
	message.which_message = MeterReader_Message_update_tag;
	message.message.update.meterId = meterId;
	message.message.update.seriesId = seriesId;
	message.message.update.currentCounterValue = currentValue;

  sendMessage(s, message);
}

void sendSettings(Stream &s, MeterReader_Settings &settings) {
    MeterReader_Message message;
    message.which_message = MeterReader_Message_settings_tag;
    message.message.settings = settings;
    
    sendMessage(s, message);
}