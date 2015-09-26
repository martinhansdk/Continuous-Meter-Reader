#include "gtest/gtest.h"
#include "arduino-mock/Arduino.h"
#include "OpticalSensor.h"

namespace testing {

	TEST(OpticalSensorTest, noticesEdges) {
  		ArduinoMock* arduinoMock = arduinoMockInstance();

  		EXPECT_CALL(*arduinoMock, pinMode(0, INPUT));
  		EXPECT_CALL(*arduinoMock, digitalRead(0))
  			.WillOnce(Return(0)) // init
  			.WillOnce(Return(0))
  			.WillOnce(Return(1)) // first rising
  			.WillOnce(Return(1))
  			.WillOnce(Return(1))
  			.WillOnce(Return(1))
  			.WillOnce(Return(0)) // first falling
  			.WillOnce(Return(0))
  			.WillOnce(Return(0))
  			.WillOnce(Return(1)) // second rising
  			.WillRepeatedly(Return(1));

		OpticalSensor sensor(0, 100, 80); // init
		
		EXPECT_EQ(0, sensor.getAmount());
		EXPECT_EQ(100, sensor.getAmount()); // first rising
		EXPECT_EQ(0, sensor.getAmount());
		EXPECT_EQ(0, sensor.getAmount());
		EXPECT_EQ(0, sensor.getAmount());
		EXPECT_EQ(80, sensor.getAmount()); // first falling
		EXPECT_EQ(0, sensor.getAmount());
		EXPECT_EQ(0, sensor.getAmount());
		EXPECT_EQ(100, sensor.getAmount()); // second rising

  		releaseArduinoMock();
	}


}