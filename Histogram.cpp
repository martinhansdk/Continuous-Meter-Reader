#include "Histogram.h"

Histogram::Histogram() {
  clear();
}

void Histogram::sample(sample_t value) {
  histogram[value]++;
  noOfSamples++;
}

void Histogram::clear() {
  noOfSamples=0;
  for(int i = MIN_SAMPLE_VALUE ; i <= MAX_SAMPLE_VALUE ; i++) {
    histogram[i]=0;
  }
}

sample_t Histogram::getPercentile(unsigned int percentile) {
  unsigned int noOfSamplesSeen=0;

  if(noOfSamples==0) {
    return 0;
  }
  for(int i = MIN_SAMPLE_VALUE ; i <= MAX_SAMPLE_VALUE ; i++) {
    
    noOfSamplesSeen+=histogram[i];
    if( float(100*noOfSamplesSeen)/float(noOfSamples) >= percentile) {
      return i;
    }
  } 

  return MAX_SAMPLE_VALUE;
}
