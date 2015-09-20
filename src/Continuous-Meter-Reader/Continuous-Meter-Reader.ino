//-*-c++-*-
/* Meter reader */

#include "Histogram.h"
#include "Meter.h"
#include "Comparator.h"
#include "MeterReader_pb.h"
#include "settings.h"
#include "Communicate.h"

void serial_write_uint32(uint32_t val);

Comparator comparator;
Meter meter;
Histogram histogram;
Settings<MeterReader_Settings> settings;

#define ADCPIN A7
const int SAMPLE_FREQUENCY = 200; // Hz
const int CALIBRATION_SECONDS = 5;
const int SAMPLE_TIME = 1000 / SAMPLE_FREQUENCY;
const int SAMPLES_TO_CALIBRATE = CALIBRATION_SECONDS*1000/SAMPLE_TIME;
const int TICKS_BETWEEN_SEND = 40; // 0.2 seconds

int calibrationSamples=SAMPLES_TO_CALIBRATE;
bool calibrationMode, calibrationDone;
bool sendValueFlag;
unsigned int ticks; // must be big enough to hold TICKS_BETWEEN_SEND
Receiver<MeterReader_Message, MeterReader_Message_fields> serialinput(Serial);

uint64_t lastSentValue = UINT64_MAX;

void setup() {
  sendValueFlag = false;
  ticks = 0;
  calibrationMode = false;
  calibrationDone = false;

  // pick the next series
  settings.load();
  settings.s.seriesId += 1;
  settings.save();

  settings.s.risingEdgeAmounts_count = 6;
  settings.s.fallingEdgeAmounts_count = 6;

  Serial.begin(57600);

  // the serial port does not work reliably right after initializing 
  // the port, this delay makes sure it's ready before we use it
  delay(500);
  sendLog(Serial, MeterReader_LogMessage_Type_NOTE, "Rebooted");
  sendSettings(Serial, settings.s);
  
  comparator.setThreshold(settings.s.threshold);
  comparator.setHysteresis(settings.s.hysteresis);
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
      calibrationMode = false;
      calibrationDone = true;
    }
  } else {
    comparator.sample(sensorValue);  

    ticks++;    
    if(ticks > TICKS_BETWEEN_SEND) {
      sendValueFlag = true;
      ticks = 0;
    }
  }


}

void loop() {
    if(sendValueFlag) {
      uint64_t currentValue;
      // protected access to variables shared with ISR BEGIN
      noInterrupts();
      sendValueFlag = 0;
      currentValue = meter.getCurrentValue();
      interrupts();
      // protected access to variables shared with ISR END

      if(currentValue != lastSentValue) {
        sendCounterUpdate(Serial, settings.s.meterId, settings.s.seriesId, currentValue);
        lastSentValue = currentValue;
      }

      if(calibrationDone) {
        calibrationDone = false;
        settings.s.threshold = comparator.getThreshold();
        settings.s.hysteresis = comparator.getHysteresis();
        settings.save();
        sendSettings(Serial, settings.s);
      }

      if(serialinput.process()) {
        switch(serialinput.message.which_message) {
          case MeterReader_Message_calibrate_tag:
            noInterrupts();
            calibrationMode = true;
            calibrationSamples=SAMPLES_TO_CALIBRATE;
            interrupts();
            break;
          case MeterReader_Message_settings_tag:
            noInterrupts();
            settings.s = serialinput.message.message.settings;
            interrupts();
            settings.save();
            sendSettings(Serial, settings.s);
            break;
          default:
            // ignore message
            break;
        }
      }
    }
}

