#include "gtest/gtest.h"
#include "arduino-mock/Arduino.h"
#include "OpticalSensor.h"

using namespace std;

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

  		OpticalSensor sensor(0); // init
          sensor.setRisingEdgeAmount(100);
          sensor.setFallingEdgeAmount(80);

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


   class ArduinoFakeForSensorTest : public ArduinoMockBase {
    int time;
    vector<string> waveform;
public:
    ArduinoFakeForSensorTest() : time(0) {};
    void setWaveform(int pin, string wavedef) { 
        if(waveform.size() <= pin) {
            waveform.resize(pin + 1);
        }
        waveform[pin] = wavedef;
    };
    void tick() { time++; }

    virtual void pinMode(uint8_t pin, uint8_t mode) { ASSERT_EQ(INPUT, mode) << "All pins should be inputs"; };
    virtual void analogWrite(uint8_t, int) {};
    virtual void digitalWrite(uint8_t, uint8_t) { FAIL() << "Should not write to outputs"; };
    virtual int digitalRead(int pin) { 
        if(pin >= waveform.size()) {
            pin = waveform.size() - 1;
        }

        size_t index = time;
        if(index >= waveform[pin].length())
            index = waveform[pin].length() - 1;

        return waveform[pin][index] == 'H' ? HIGH : LOW; 
    };
    virtual int analogRead(int) { return 0; };
    virtual void delay(int) { FAIL() << "not implemented"; };
    virtual unsigned long millis() { return 0; };
};

class OpticalSensorCalibrationTest : public ::testing::Test {
    ArduinoFakeForSensorTest mock;
protected:
    virtual void SetUp() {
        setArduinoMockInstance(&mock);
    }

    virtual void TearDown() {
        setArduinoMockInstance(NULL);
    }

    void setWaveform(int pin, string wavedef) { mock.setWaveform(pin, wavedef); };
    void tick() { mock.tick(); }

};

  /* Test calibration. We will do this with three sensors which during constant flow 
  experience the following pattern:

  sensor 1   L  H  H  H  H  H  H  L  L  L  L  H

  sensor 2   L  L  L  H  H  H  H  H  H  L  L  L

  sensor 3   L  L  L  L  L  H  H  H  H  H  L  L  

      tick   0  1  2  3  4  5  6  7  8  9  A  B  

  So all edge events take two ticks since the previous event, except sensor 1
  rising and sensor 3 falling. Which each take only one tick.
  */
  TEST_F(OpticalSensorCalibrationTest, calibration) {
    setWaveform(1, "LHHHHHHLLLLH");
    setWaveform(2, "LLLHHHHHHLLL");
    setWaveform(3, "LLLLLHHHHHLL"); 

    OpticalSensor sensors[] = { 
        OpticalSensor(1),
        OpticalSensor(2),
        OpticalSensor(3)
    };

    OpticalSensorCalibrator calibrator(sensors);
    calibrator.start(3, 1001, 1);

    int time_left = 12;
    bool done = false;
    while(time_left--) {
        if(calibrator.tick()) {
            done = true;
            break;
        }
        tick();
    }

    ASSERT_TRUE(done);
    EXPECT_EQ(100, sensors[0].getRisingEdgeAmount());
    EXPECT_EQ(201, sensors[0].getFallingEdgeAmount());
    EXPECT_EQ(200, sensors[1].getRisingEdgeAmount());
    EXPECT_EQ(200, sensors[1].getFallingEdgeAmount());
    EXPECT_EQ(200, sensors[2].getRisingEdgeAmount());
    EXPECT_EQ(100, sensors[2].getFallingEdgeAmount());

    }
}