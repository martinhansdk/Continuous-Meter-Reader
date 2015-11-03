#include "gtest/gtest.h"
#include "Meter.h"

namespace testing {

class MeterTest : public ::testing::Test {
  protected:
    Meter meter;

    void callIncrementNTimes(unsigned int times) {
      for(unsigned int i = 0; i < times ; i++) {
        meter.increment();
      }
    }
};

TEST_F(MeterTest, countsCorrectly) {
  EXPECT_EQ(0u, meter.getCurrentValue());

  callIncrementNTimes(20);
  EXPECT_EQ(20u, meter.getCurrentValue());

  callIncrementNTimes(55);
  EXPECT_EQ(75u, meter.getCurrentValue());

  callIncrementNTimes(100);
  EXPECT_EQ(175u, meter.getCurrentValue());
}

}
