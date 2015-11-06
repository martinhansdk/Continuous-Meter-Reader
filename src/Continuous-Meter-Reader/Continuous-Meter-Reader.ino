//-*-c++-*-
/* Meter reader */

#include <SPI.h>
#include "Histogram.h"
#include "Meter.h"
#include "Comparator.h"
#include "MeterReader_pb.h"
#include "OpticalSensor.h"
#include "settings.h"
#include "CommunicateSerial.h"
#include "CommunicateWireless.h"
#include "RadioMessageSender.h"
#include "crc32.h"

Settings<MeterReader_Settings> settings;

#define POWER_ENABLE_PIN A1

// the power enable pin is active low because it is connected to a P-MOS transistor
inline void power_leds_on() {
  digitalWrite(POWER_ENABLE_PIN, LOW);
}

inline void power_leds_off() {
  digitalWrite(POWER_ENABLE_PIN, HIGH);
}

// Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(9, 10);
RadioMessageSender radioSender(radio);

const char* addresses[] = {"meterS", "meterR"};

Meter meter;
// digital sensors
OpticalSensor digitalSensors[] = {
  OpticalSensor(8),
  OpticalSensor(7),
  OpticalSensor(6),
  OpticalSensor(5),
  OpticalSensor(4),
  OpticalSensor(3)
};
OpticalSensorCalibrator calibrator(digitalSensors);

const int SAMPLE_FREQUENCY = 200; // Hz
const int SAMPLE_TIME = 1000 / SAMPLE_FREQUENCY; // ms
const int TICKS_BETWEEN_SEND = 200; // 1 second

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

  pinMode(POWER_ENABLE_PIN, OUTPUT);
  power_leds_off();

  // pick the next series
  settings.load();
  settings.s.seriesId += 1;
  settings.save();

  // sanitize the number of digital sensors
  if(settings.s.fallingEdgeAmounts_count > 6) {
    settings.s.risingEdgeAmounts_count = 0;
  }

  settings.s.fallingEdgeAmounts_count = settings.s.risingEdgeAmounts_count;

  for(int i = 0 ; i < settings.s.risingEdgeAmounts_count ; i++) {
    digitalSensors[i].setRisingEdgeAmount(settings.s.risingEdgeAmounts[i]);
    digitalSensors[i].setFallingEdgeAmount(settings.s.fallingEdgeAmounts[i]);
  }

  Serial.begin(57600);

  // the serial port does not work reliably right after initializing
  // the port, this delay makes sure it's ready before we use it
  delay(500);
  sendLog(Serial, MeterReader_LogMessage_Type_NOTE, "Rebooted");
  sendSettings(Serial, settings.s);

  radio.begin();
  radio.setPALevel(RF24_PA_MAX);

  //myAddress = baseAddress + settings.s.meterId;
  radio.openWritingPipe((const uint8_t*)addresses[1]);
  radio.openReadingPipe(1, (const uint8_t*)addresses[0]);
  radio.enableDynamicPayloads();
  radio.stopListening();
  radio.powerDown();

  radioSender.begin(settings.s.meterId, settings.s.seriesId);

// initialize timer1
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 16000000 / 64 / SAMPLE_FREQUENCY; // compare match register 16MHz/64/200Hz
  TCCR1B |= (1 << WGM12);   // CTC mode (Clear timer on compare match)
  TCCR1B |= (1 << CS11) | (1 << CS10);  // 64 prescaler
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  interrupts();             // enable all interrupts
}

ISR(TIMER1_COMPA_vect) {        // timer compare interrupt service routine
  power_leds_on();
  if(calibrationMode) {
    if(calibrator.tick()) {
      calibrationMode = false;
      calibrationDone = true;
    }
  } else {
    int change = 0;
    for(int sensor = 0; sensor < settings.s.risingEdgeAmounts_count ; sensor++) {
      change += digitalSensors[sensor].getAmount();
    }
    power_leds_off();
    meter.add(change);

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

    if(true || currentValue != lastSentValue) {
      if(settings.s.communicationChannel == MeterReader_Settings_CommunicationChannel_WIRELESS) {
        radio.powerUp();
        sendCounterUpdateByRadio(radioSender, settings.s.meterId, settings.s.seriesId, currentValue);
        radio.powerDown();
      } else {
        sendCounterUpdate(Serial, settings.s.meterId, settings.s.seriesId, currentValue);
      }
      lastSentValue = currentValue;
    }

    if(calibrationDone) {
      calibrationDone = false;
      noInterrupts();

      for(int i = 0 ; i < settings.s.fallingEdgeAmounts_count; i++) {
        settings.s.fallingEdgeAmounts[i] = digitalSensors[i].getFallingEdgeAmount();
        settings.s.risingEdgeAmounts[i] = digitalSensors[i].getRisingEdgeAmount();
      }

      settings.save();
      interrupts();
      sendSettings(Serial, settings.s);
    }
  }

  if(serialinput.process()) {
    switch(serialinput.message.which_message) {
    case MeterReader_Message_calibrate_tag:
      noInterrupts();
      calibrationMode = true;
      calibrator.start(settings.s.fallingEdgeAmounts_count, 1000, 1);

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

