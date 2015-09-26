
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
