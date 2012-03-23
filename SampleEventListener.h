
/**
 SampleEventListener interface that must be implemented by all
classes that want to subscribe to sample events.

*/

#ifndef _SAMPLEEVENTLISTENER_H
#define _SAMPLEEVENTLISTENER_H

typedef uint16_t sample_t;

const int MAX_SAMPLE_VALUE=1023;
const int MIN_SAMPLE_VALUE=0;

class SampleEventListener {
  virtual ~SampleEventListener() {};
  virtual void sample(sample_t value) = 0;
}

#endif
