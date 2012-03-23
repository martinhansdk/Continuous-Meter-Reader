/**
The Meter class implements a counter which keeps track
of how many real life quantities (meters, kWh, etc) have 
been measured so far.

*/

#ifndef _METER_H
#define _METER_H

#include "UnitIncrementListener.h"

class Meter : public UnitIncrementListener {
  Meter();
  virtual void increment();
  void setQuantityPerIncrement(float amount); // eg. Litres per increment
  float getCurrentAmount();
}

#endif
