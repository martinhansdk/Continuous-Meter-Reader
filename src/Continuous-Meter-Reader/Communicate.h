#ifndef _COMMUNICATE_H
#define _COMMUNICATE_H

#include <Stream.h>
#include <MeterReader_pb.h>

void sendCounterUpdate(Stream &s, int32_t meterId, uint32_t seriesId, uint64_t currentValue);
void sendSettings(Stream &s, MeterReader_Settings &settings);

#endif