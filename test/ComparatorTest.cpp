#include "gmock/gmock.h"
#include "MockUnitIncrementListener.h"
#include "Comparator.h"

#define _USE_MATH_DEFINES
#include <cmath>

namespace testing {

class ComparatorTest : public ::testing::Test {
  protected:

    Comparator comparator;
    MockUnitIncrementListener listener;


    virtual void SetUp() {
      comparator.addUnitIncrementListener(&listener);

      comparator.setThreshold(50);
      comparator.setHysteresis(5);
    }

};

TEST_F(ComparatorTest, respectsThresholdSetting) {
  EXPECT_CALL(listener, increment()).Times(2);

  comparator.sample(0);
  comparator.sample(1);
  comparator.sample(2);
  comparator.sample(49);
  comparator.sample(54);
  comparator.sample(55);
  comparator.sample(56); // first unit
  comparator.sample(54);
  comparator.sample(51);
  comparator.sample(80); // nothing yet, still didn't go under the lower threshold...
  comparator.sample(44);
  comparator.sample(80); // second unit
}


TEST_F(ComparatorTest, canCountsEdgesOnASineWave) {
  const int CYCLES = 7;
  const float TIME_INCREMENT = 2.0 * M_PI / 100;
  const float AMPLITUDE = 20;
  const int OFFSET = 47;

  EXPECT_CALL(listener, increment()).Times(CYCLES);

  // generate a sine wave slightly under the Comparator's
  // threshold
  for(float t = 0; t < CYCLES ; t += TIME_INCREMENT) {
    comparator.sample(OFFSET + int(AMPLITUDE * sin(2 * M_PI * float(t))));
  }
}

}
