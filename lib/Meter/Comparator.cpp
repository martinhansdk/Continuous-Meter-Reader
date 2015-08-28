#include "Comparator.h"

Comparator::Comparator() : 
  threshold(MID_SAMPLE_VALUE), 
  hysteresis(MAX_SAMPLE_VALUE/10),
  state(0),
  noOfListeners(0) {
}

void Comparator::notifyListeners() {
  for(unsigned int i=0 ; i < noOfListeners ; i++) {
    listeners[i]->increment();
  }
}

void Comparator::sample(sample_t value) {
  if(state==0 && value > threshold+hysteresis) {
    state=1;
    notifyListeners();
  } else if(state==1 && value < threshold-hysteresis) {
    state=0;
  }
}

void Comparator::addUnitIncrementListener(UnitIncrementListener *listener) {
  if(noOfListeners < MAX_LISTENERS) {
    listeners[noOfListeners++] = listener;
  }
}

void Comparator::setThreshold(sample_t newThreshold) {
  threshold = newThreshold;
}

void Comparator::setHysteresis(sample_t newHysteresis) {
  hysteresis = newHysteresis;
}

