#include "gmock/gmock.h"
#include "MockUnitIncrementListener.h"

namespace testing {

  class ComparatorTest : public ::testing::Test {
  protected:

    virtual SetUp() {
      Comparator comparator;
      MockUnitIncrementListener listener;
      comparator.addUnitIncrementListener(&listener);
      
      comparator.setThreshold(50);
      comparator.setHysteresis(5);
    }
    
  }

  TEST(ComparatorTest, respectsThresholdSetting) {
    EXPECT_CALL(listener, increment()).Times(2);

    comparator.sample(0);
    comparator.sample(1);
    comparator.sample(2);
    comparator.sample(49)
    comparator.sample(54);
    comparator.sample(55);
    comparator.sample(56); // first unit
    comparator.sample(54);
    comparator.sample(51);
    comparator.sample(80); // nothing yet, still didn't go under the lower threshold...
    comparator.sample(45);
    comparator.sample(80); // second unit
  }

  TEST(ComparatorTest, canCountsEdgesOnASineWave) {
    const int CYCLES=7;
    const float TIME_INCREMENT=2.0*PI/100;
    const float AMPLITUDE=20;
    const int OFFSET=47;

    EXPECT_CALL(listener, increment()).Times(CYCLES);

    // generate a sine wave slightly under the Comparator's
    // threshold
    for(float t=0; t < CYCLES ; t+=TIME_INCREMENT) {
      comparator.sample(OFFSET + int(AMPLITUDE*sin(2*PI*float(t))));
    }
  }

}
