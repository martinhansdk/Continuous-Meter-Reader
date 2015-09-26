#include "gtest/gtest.h"
#include "Histogram.h"

namespace testing {

  class HistogramTest : public ::testing::Test {
  protected:
    Histogram histogram;

    void sampleValueNTimes(sample_t value, unsigned int times) {
      for(unsigned int i = 0; i < times ; i++) {
    	  histogram.sample(value);
      }
    }
  };

  TEST_F(HistogramTest, calculatesPercentilesCorrectlyOnAFlatDistribution) {
    for(sample_t i=MIN_SAMPLE_VALUE; i <=MAX_SAMPLE_VALUE ; i++) {
      histogram.sample(i);
    }

    EXPECT_EQ(MIN_SAMPLE_VALUE, histogram.getPercentile(0));
    EXPECT_NEAR(MAX_SAMPLE_VALUE, histogram.getPercentile(100), 3);
    EXPECT_NEAR((MAX_SAMPLE_VALUE-MIN_SAMPLE_VALUE)/2, histogram.getPercentile(50), 3);
  }

  TEST_F(HistogramTest, isUsefulForCalibration) {
    
    /* sample the following histogram shape

       
                *      *
                **     *
                ***   ***
             *  ****  *** *
         ----------------------->
     */

    sampleValueNTimes(200, 1);
    sampleValueNTimes(205, 4);
    sampleValueNTimes(206, 3);
    sampleValueNTimes(207, 2);
    sampleValueNTimes(208, 1);
    sampleValueNTimes(215, 2);
    sampleValueNTimes(216, 4);
    sampleValueNTimes(217, 2);
    sampleValueNTimes(223, 1);

    EXPECT_NEAR(205, histogram.getPercentile(15), 1);
    EXPECT_NEAR(216, histogram.getPercentile(90), 1);
  }

  TEST_F(HistogramTest, isUsefulForCalibration2) {
    
    /* sample the following histogram shape

       
                *      *
                *      *
                *      *
                *      *
         ----------------------->
     */

    for(int i=0 ; i < 500 ; i++) {
      histogram.sample(223);
      histogram.sample(226);      
    }

    EXPECT_NEAR(220, histogram.getPercentile(25), 0);
    EXPECT_NEAR(224, histogram.getPercentile(75), 0);
  }

} // namespace
