
/* Meter reader */

#include "Histogram.h"
#include "Meter.h"
#include "Comparator.h"

Comparator comparator;
Meter meter;
Histogram histogram;

#define ADCPIN A0
const int SAMPLE_FREQUENCY = 100;
const int CALIBRATION_SECONDS = 5;
const int SAMPLE_TIME = 1000 / SAMPLE_FREQUENCY;
const int SAMPLES_TO_CALIBRATE = CALIBRATION_SECONDS*1000/SAMPLE_TIME;

int calibrationSamples=0;
int measureSamples=0;

void setup() {
  Serial.begin(9600);
  
  comparator.addUnitIncrementListener(&meter);  
}

// the loop routine runs over and over again forever:
void loop() {
  delay(10); // 100 Hz
  int sensorValue = analogRead(ADCPIN);

  if(calibrationSamples < SAMPLES_TO_CALIBRATE) {
    histogram.sample(sensorValue);
    calibrationSamples++;
  } else if(calibrationSamples == SAMPLES_TO_CALIBRATE) {
    int low = histogram.getPercentile(25);
    int high = histogram.getPercentile(75);
    int hysteresis = (high-low)/2;
    int threshold=(high+low)/2;    
    comparator.setHysteresis(hysteresis);
    comparator.setThreshold(threshold);
    
    Serial.print("low="); Serial.println(float(low)/1024.0*5.0);
    Serial.print("high="); Serial.println(float(high)/1024.0*5.0);
    Serial.print("threshold="); Serial.println(float(threshold)/1024.0*5.0);
    Serial.print("hysteresis="); Serial.println(float(hysteresis)/1024.0*5.0
    );
    
    calibrationSamples++;
  } else {
    comparator.sample(sensorValue);  
    measureSamples++;
    
    if(measureSamples%25==0) {
      Serial.println(meter.getCurrentAmount());
    }
  }
}

