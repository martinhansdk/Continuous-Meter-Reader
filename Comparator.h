/**
The comparator class implements a classic analog comparator with hysteresis.

*/

#ifndef _COMPARATOR_H
#define _COMPARATOR_H

#include "SampleEventListener.h"
#include "UnitIncrementListener.h"

class Comparator : public SampleEventListener {
  Comparator();
  virtual void sample(sample_t value);
  void addUnitIncrementListener(UnitIncrementListener *listener);
  void setThreshold(sample_t value);
  void setHysteresis(sample_t value);
}

#endif
