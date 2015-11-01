#include "CommunicateWireless.h"


#undef true
#undef false

bool radio_write_callback(pb_ostream_t *stream, const uint8_t *buf, size_t count) {
  RadioMessageSender *sender = (RadioMessageSender*) stream->state;
  bool ok = sender->write(buf, count, false);

  return ok;
}

void sendMessageByRadio(RadioMessageSender &sender, MeterReader_Message &message) {
  sender.startMessage();
  pb_ostream_t stream = {&radio_write_callback, &sender, 255, 0, ""};
  bool ok = pb_encode(&stream, MeterReader_Message_fields, &message); // send data

  if(ok) {
    sender.write((const uint8_t*)"", 0, true); // signal end of message
  }
}

void sendCounterUpdateByRadio(RadioMessageSender &s, int32_t meterId, uint32_t seriesId, uint64_t currentValue) {
  MeterReader_Message message;
  message.which_message = MeterReader_Message_update_tag;
  message.message.update.meterId = meterId;
  message.message.update.seriesId = seriesId;
  message.message.update.currentCounterValue = currentValue;

  sendMessageByRadio(s, message);
}