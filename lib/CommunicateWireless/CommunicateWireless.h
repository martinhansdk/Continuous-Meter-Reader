#ifndef _COMMUNICATEWIRELESS_H
#define _COMMUNICATEWIRELESS_H

#include "pb_encode.h"
#include <MeterReader_pb.h>
#include <RadioMessageSender.h>

void sendMessageByRadio(RadioMessageSender &sender, MeterReader_Message &message);
void sendCounterUpdateByRadio(RadioMessageSender &s, int32_t meterId, uint32_t seriesId, uint64_t currentValue);

#endif