#include "EEPROM.h"
#include <vector>
#include <iostream>

EEPROMDummy EEPROM;

using namespace std;

uint8_t EEPROMDummy::read(int pos) {
  if(pos >= 0 && pos < (int)(sizeof(storage) / sizeof(uint8_t))) {
    return storage[pos];
  } else {
    return 0;
  }
}

void EEPROMDummy::write(int pos, uint8_t val) {
  if(pos >= 0 && pos < (int)(sizeof(storage) / sizeof(uint8_t))) {
    storage[pos] = val;
  }
}