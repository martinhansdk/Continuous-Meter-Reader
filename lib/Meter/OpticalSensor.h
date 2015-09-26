
#include "Arduino.h"

class OpticalSensor {
    int pin;
    int risingAmount, fallingAmount;
    bool state;
public:
    OpticalSensor(int pin, int risingAmount, int fallingAmount) : pin(pin), risingAmount(risingAmount), fallingAmount(fallingAmount) {
        pinMode(pin, INPUT);
        state = digitalRead(pin);
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
