#include "RF24.h"

// interleave chunks from two source radios onto one receiver radio
void interleaveRadioTransmissions(RF24 &target, RF24 &source1, RF24 &source2) {
  vector<string>::const_iterator s1 = source1.chunks.begin();
  vector<string>::const_iterator s2 = source2.chunks.begin();

  while(s1 != source1.chunks.end() && s2 != source2.chunks.end()) {
    if(s1 != source1.chunks.end()) {
      target.chunks.push_back(*s1);
      ++s1;
    }
    if(s2 != source1.chunks.end()) {
      target.chunks.push_back(*s2);
      ++s2;
    }
  }
}