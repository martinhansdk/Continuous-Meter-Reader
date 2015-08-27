//-*-c++-*-
/* Meter reader */

#include "Histogram.h"
#include "Meter.h"
#include "Comparator.h"
#include "CounterUpdate_pb.h"
#include "pb_encode.h"
#include "crc32.h"


#define htonl(x) ( ((x)<<24 & 0xFF000000UL) | \
                   ((x)<< 8 & 0x00FF0000UL) | \
                   ((x)>> 8 & 0x0000FF00UL) | \
                   ((x)>>24 & 0x000000FFUL) )


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

int32_t meterId = 1;
int32_t seriesId = 1;

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
      uint64_t currentValue;
      // protected access to variables shared with ISR BEGIN
      noInterrupts();
      sendValueFlag = 0;
      currentValue = meter.getCurrentValue();
      interrupts();
      // protected access to variables shared with ISR END

      {
        MeterReader_CounterUpdate mymessage = {meterId, seriesId, currentValue};
        uint8_t buffer[MeterReader_CounterUpdate_size];
        
        pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
        pb_encode(&stream, MeterReader_CounterUpdate_fields, &mymessage);
        Serial.write("AA");
        serial_write_uint32(stream.bytes_written); 
        Serial.write(buffer, stream.bytes_written);
        uint32_t crc = crc_array(buffer, stream.bytes_written);
        serial_write_uint32(crc);
      }
    }
}

void serial_write_uint32(uint32_t val) {
  uint8_t bytes[4];
  val = htonl(val); // convert to network byte order
  bytes[0] = (val >> 24) & 0xFF;
  bytes[1] = (val >> 16) & 0xFF;
  bytes[2] = (val >> 8) & 0xFF;
  bytes[3] = val & 0xFF;
  
  Serial.write(bytes, 4);
}
