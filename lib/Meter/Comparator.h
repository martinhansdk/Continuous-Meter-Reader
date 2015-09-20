/**
The comparator class implements a classic analog comparator with hysteresis.

*/

#ifndef _COMPARATOR_H
#define _COMPARATOR_H

#include "SampleEventListener.h"
#include "UnitIncrementListener.h"

class Comparator : public SampleEventListener {
  static const unsigned int MAX_LISTENERS=2;
  sample_t threshold;
  sample_t hysteresis;
  uint8_t state;

  unsigned int noOfListeners;
  UnitIncrementListener* listeners[MAX_LISTENERS];

  void notifyListeners();
public:
  Comparator();
  virtual void sample(sample_t value);
  void addUnitIncrementListener(UnitIncrementListener *listener);
  void setThreshold(sample_t value);
  void setHysteresis(sample_t value);
  sample_t getThreshold() { return threshold; };
  sample_t getHysteresis() { return hysteresis; };
};

#endif
