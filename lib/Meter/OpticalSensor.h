
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
        bool currentSensorState = pinRead();
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


OpticalSensor sensors[] = {
    OpticalSensor(D1, 100, 100),
    OpticalSensor(D2, 100, 100), 
    OpticalSensor(D3, 100, 100), 
    OpticalSensor(D4, 100, 100), 
    OpticalSensor(D5, 100, 100), 
    OpticalSensor(D6, 100, 100)
}

const int SENSORS = sizeof(sensors) / sizeof(OpticalSensor);

int change = 0;
for(int sensor = 0; sensor < SENSORS ; sensor++) {
    change += sensors[sensor].getAmount();
}

