//-*-c++-*-
/* Meter reader */

#include "Histogram.h"
#include "Meter.h"
#include "Comparator.h"

Comparator comparator;
Meter meter;
Histogram histogram;

#define ADCPIN A0
const int SAMPLE_FREQUENCY = 200; // Hz
const int CALIBRATION_SECONDS = 5;
const int SAMPLE_TIME = 1000 / SAMPLE_FREQUENCY;
const int SAMPLES_TO_CALIBRATE = CALIBRATION_SECONDS*1000/SAMPLE_TIME;
const int TICKS_BETWEEN_SEND = 40; // 0.2 seconds

int calibrationSamples=SAMPLES_TO_CALIBRATE;
int calibrationMode;
int sendValueFlag;
unsigned int ticks; // must be big enough to hold TICKS_BETWEEN_SEND

void setup() {
  sendValueFlag = 0;
  ticks = 0;
  calibrationMode = 1;

  Serial.begin(9600);
  
  comparator.addUnitIncrementListener(&meter);  

 // initialize timer1 
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 16000000/64/SAMPLE_FREQUENCY;  // compare match register 16MHz/64/200Hz
  TCCR1B |= (1 << WGM12);   // CTC mode (Clear timer on compare match)
  TCCR1B |= (1 << CS11) | (1 << CS10);  // 64 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  interrupts();             // enable all interrupts
}

ISR(TIMER1_COMPA_vect)          // timer compare interrupt service routine
{

  int sensorValue = analogRead(ADCPIN);

  if(calibrationMode) {
    if(calibrationSamples > 0) {
      histogram.sample(sensorValue);
      calibrationSamples--;
    } else if(calibrationSamples == 0) {
      int low = histogram.getPercentile(25);
      int high = histogram.getPercentile(75);
      int hysteresis = (high-low)/2;
      int threshold=(high+low)/2;    
      comparator.setHysteresis(hysteresis);
      comparator.setThreshold(threshold);    
      calibrationMode = 0;
    }
  } else {
    comparator.sample(sensorValue);  

    ticks++;    
    if(ticks > TICKS_BETWEEN_SEND) {
      sendValueFlag = 1;
      ticks = 0;
    }
  }


}

void loop() {
    if(sendValueFlag) {
      unsigned long currentValue;
      // protected access to variables shared with ISR BEGIN
      noInterrupts();
      sendValueFlag = 0;
      currentValue = meter.getCurrentValue();
      interrupts();
      // protected access to variables shared with ISR END

      Serial.print("<");
      Serial.print(currentValue);
      Serial.print(">");
    }
}

