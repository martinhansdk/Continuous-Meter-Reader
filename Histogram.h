/**
The Histogram class implements keeping statistics for how often 
each of the possible sample values occurred. It can be used
for automatically calibrating the comparator settings.

*/

#ifndef _HISTOGRAM_H
#define _HISTOGRAM_H

#include "SampleEventListener.h"

class Histogram : public SampleEventListener {
  Histogram();
  virtual void sample(sample_t value);
  void clear();
  sample_t getPercentile(unsigned int percentile);
}

#endif
