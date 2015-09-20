#include <stdint.h>
#include "Communicate.h"
#include "crc32.h"
#include "pb_encode.h"
#include "pb_decode.h"

void write_uint32(Stream &s, uint32_t val) {
  uint32_t le_val = htole32(val);
  
  s.write((uint8_t*)&le_val, 4);
}

void sendError(Stream &s, const char* errmsg) {
  sendLog(s, MeterReader_LogMessage_Type_ERROR, errmsg);
}

bool write_callback(pb_ostream_t *stream, const uint8_t *buf, size_t count)
{
  StreamState *state = (StreamState*) stream->state;

  size_t sent = state->stream.write(buf, count);
  state->crc_add(buf, count);

  return (sent == count);
}

bool read_callback(pb_istream_t *stream, uint8_t *buf, size_t count)
{
   StreamState *state = (StreamState*) stream->state;

   if (buf == NULL)
   {
       while (count--) 
          state->stream.read();
       return true;
   }

  size_t got = state->stream.readBytes((char*)buf, count);
  state->crc_add(buf, count);

  return (got == count);
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
  StreamState state(s);
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
    if(message.message.settings.risingEdgeAmounts_count > 6) {
      message.message.settings.risingEdgeAmounts_count = 0;
    }
    if(message.message.settings.fallingEdgeAmounts_count > 6) {
      message.message.settings.fallingEdgeAmounts_count = 0;
    }
    
    sendMessage(s, message);
}

void sendLog(Stream &s, MeterReader_LogMessage_Type type, const char* text) {
    MeterReader_Message message;
    message.which_message = MeterReader_Message_log_tag;
    message.message.log.type = type;
    strncpy ( message.message.log.text, text, sizeof(message.message.log.text)-1 );
    message.message.log.text[sizeof(message.message.log.text)-1] = '\0';

    sendMessage(s, message);
}