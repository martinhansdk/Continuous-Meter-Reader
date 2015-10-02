
#include "Arduino.h"


class OpticalSensor {
    int pin;
    int risingAmount, fallingAmount;
    bool state;
public:
    OpticalSensor(int pin) : pin(pin), risingAmount(0), fallingAmount(0) {
        pinMode(pin, INPUT);
        state = digitalRead(pin);
    }

    void setRisingEdgeAmount(int amount) {
        risingAmount = amount;
    }

    void setFallingEdgeAmount(int amount) {
        fallingAmount = amount;
    }

    int getRisingEdgeAmount() {
        return risingAmount;
    }

    int getFallingEdgeAmount() {
        return fallingAmount;
    }

    int getAmount() {
        bool currentSensorState = digitalRead(pin);
        if(currentSensorState != state) {
            state = currentSensorState;

            if(currentSensorState) {
                return risingAmount;
            } else {
                return fallingAmount;
            }
        }
        return 0;
    }

};

/** Calibration of a set of optical digital sensors works as follows:

  We know that once we have seen all rising and falling edge on all sensors
  once, we know that we have consumed totalSum amount (of water, electricity,
  whatever). Because the refelective, moving part of the meter has a different
  size than the non-reflective part, seeing a rising edge on a given sensor
  will mean a different amount was consumed than if we see a falling edge.

  During calibration, constant flow must be present. During constant flow a
  given edge event means the same was consumed as for any other given edge
  event. If we measure the time since the previous edge event then we can
  calculate the amount consumed in that time as the fraction of time spent out
  of the total time is the same as the fraction of totalSum consumed.

  Algorithm:

  1. Assume that the consumed amount flows at a constant, positive rate
  2. Wait for the first edge - rising or falling on any one of the sensors
  3. Now wait for rising and falling edge on each of the sensors and measure 
     the time since the previous edge, t[sensor,edge]
  4. When all edges have occurred divide the totalSum value to each of the 
     edges according to the following expression:

       amount[sensor,edge] = totalSum * t[sensor,edge] / ticksSinceFirstEdge

     where ticksSinceFirstEdge is the the sum of all t[sensor,edge]

    This code does not handle the case where two edges happen at the same time.
*/



const int FALLING_EDGE = -1;
const int RISING_EDGE = -2;

class OpticalSensorCalibrator {
    int numberOfSensors;
    OpticalSensor *sensors;
    uint16_t totalSum;
    uint16_t ticksSinceFirstEdge;
    uint16_t ticksSinceLastEdge;
    bool waitingForFirstEdge;
    uint16_t remainingEdges;

    uint16_t fallingTimeTicks[6];
    uint16_t risingTimeTicks[6];

    int calculateEdgeAmount(uint16_t timeSincePreviousEdge) {
        return 0.5 + float(totalSum) * float(timeSincePreviousEdge) / float(ticksSinceFirstEdge);
    }

public:

    /** Maximum amount of sensors supported is 6.
    */
    OpticalSensorCalibrator(OpticalSensor *sensors) : sensors(sensors) {
    }
    

    /** sum is the amount that one complete round should equal.
    rounds is the number of times we should see each edge before calibration is over
    */
    void start(int num_sensors, uint16_t sum, uint8_t rounds) {
        numberOfSensors = num_sensors;
        totalSum = sum;
        ticksSinceFirstEdge = 0;
        remainingEdges = numberOfSensors * rounds * 2;
        waitingForFirstEdge = true;

        // Set the rising and falling edge amounts to marker values so we can
        // know which occurred. This overrides all previous calibration but
        // that is okay since we are going to do this at the end anyhow
        for(int i = 0 ; i < numberOfSensors ; i++) {
            sensors[i].setFallingEdgeAmount(FALLING_EDGE);
            sensors[i].setRisingEdgeAmount(RISING_EDGE);
            fallingTimeTicks[i] = 0;
            risingTimeTicks[i] = 0;
        }
    }

    // call this at regular intervals, returns true when calibration is done
    bool tick() {
        ticksSinceFirstEdge++;
        ticksSinceLastEdge++;

        // look for an edge
        int sensor = -1;
        int edge = 0;
        for(int s = 0 ; s < numberOfSensors ; s++) {
            int e = sensors[s].getAmount();

            if(e != 0) {
                sensor = s;
                edge = e;
            }
        }

        if(sensor == -1) {
            // no edge found
            return false;
        }

        if(waitingForFirstEdge) {
            waitingForFirstEdge = false;
            ticksSinceFirstEdge = 0;
            ticksSinceLastEdge = 0;
            return false;
        }

        if(edge == FALLING_EDGE) {
            fallingTimeTicks[sensor] += ticksSinceLastEdge;
        } else {
            risingTimeTicks[sensor] += ticksSinceLastEdge;
        }

        remainingEdges--;
        if(remainingEdges == 0) {

            uint16_t actualSum = 0;
            for(int i = 0 ; i < numberOfSensors ; i++) {
                uint16_t falling = calculateEdgeAmount(fallingTimeTicks[i]);
                uint16_t rising = calculateEdgeAmount(risingTimeTicks[i]);
                actualSum += falling + rising;
                sensors[i].setFallingEdgeAmount(falling);
                sensors[i].setRisingEdgeAmount(rising);
            }

            // make sure we hit totalSum exactly, correct any rounding error:
            int16_t correction = totalSum - actualSum;
            sensors[0].setFallingEdgeAmount(sensors[0].getFallingEdgeAmount() + correction);

            return true; // done!
        }

        ticksSinceLastEdge = 0;
        return false;
    }
};