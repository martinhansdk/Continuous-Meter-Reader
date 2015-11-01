/**
The Meter class implements a counter which keeps track
of how many real life quantities (meters, kWh, etc) have
been measured so far.

*/

#ifndef _METER_H
#define _METER_H

#include <stdint.h>
#include "UnitIncrementListener.h"

class Meter : public UnitIncrementListener {
    uint64_t count;
  public:
    Meter();
    virtual void increment();
    virtual void add(uint64_t amount);
    uint64_t getCurrentValue();
};

#endif
