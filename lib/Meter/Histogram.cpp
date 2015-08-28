#include "Histogram.h"

const int Histogram::SCALE;

Histogram::Histogram() {
  clear();
}

void Histogram::sample(sample_t value) {
  histogram[sampleValueToIndex(value)]++;
  noOfSamples++;
}

void Histogram::clear() {
  noOfSamples=0;
  for(unsigned int i = sampleValueToIndex(MIN_SAMPLE_VALUE) ; i <= sampleValueToIndex(MAX_SAMPLE_VALUE) ; i++) {
    histogram[i]=0;
  }
}

sample_t Histogram::getPercentile(unsigned int percentile) {
  unsigned int noOfSamplesSeen=0;

  if(noOfSamples==0) {
    return 0;
  }
  for(unsigned int i = sampleValueToIndex(MIN_SAMPLE_VALUE) ; i <= sampleValueToIndex(MAX_SAMPLE_VALUE) ; i++) {
    
    noOfSamplesSeen+=histogram[i];
    if( float(100*noOfSamplesSeen)/float(noOfSamples) >= percentile) {
      return indexToSampleValue(i);
    }
  } 

  return MAX_SAMPLE_VALUE;
}
