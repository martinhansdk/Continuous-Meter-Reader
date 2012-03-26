#include "Comparator.h"

Comparator::Comparator() : 
  threshold(MID_SAMPLE_VALUE), 
  hysteresis(MAX_SAMPLE_VALUE/10),
  state(0) {
}

void Comparator::sample(sample_t value) {
// FIXME
}

void Comparator::addUnitIncrementListener(UnitIncrementListener *listener) {
  // FIXME
}

void Comparator::setThreshold(sample_t newThreshold) {
  threshold = newThreshold;
}

void Comparator::setHysteresis(sample_t newHysteresis) {
  hysteresis = newHysteresis;
}

