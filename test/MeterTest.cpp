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
  }

  TEST_F(MeterTest, countsCorrectly) {
    Meter meter;
    const float LITRES_PER_INCREMENT = 1.0/100; // 100 increments per Litre
    const float ACCURACY=0.001;

    meter.setQuantityPerUnit(LITRES_PER_INCREMENT);

    EXPECT_NEAR(0.0, meter.getAmount(), ACCURACY);

    callIncrementNTimes(20);
    EXPECT_NEAR(0.2, meter.getAmount(), ACCURACY);

    callIncrementNTimes(55);
    EXPECT_NEAR(0.75, meter.getAmount(), ACCURACY);

    callIncrementNTimes(100);
    EXPECT_NEAR(1.75, meter.getAmount(), ACCURACY);
  }

}
