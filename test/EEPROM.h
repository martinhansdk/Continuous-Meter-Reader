#ifndef _EEPROM_H
#define _EEPROM_H

#include <stdint.h>
#include <vector>

// define a class that behaves like the arduino EEPROM library for testing

class EEPROMDummy {
    uint8_t storage[1024];

  public:
    uint8_t read(int pos);
    void write(int pos, uint8_t val);
};

extern EEPROMDummy EEPROM;

#endif