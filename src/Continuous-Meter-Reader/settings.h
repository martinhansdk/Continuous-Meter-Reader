#ifndef SETTINGS_H
#define SETTINGS_H

#include <EEPROM.h>

// ID of the settings block
#define CONFIG_VERSION "me0"

// Tell it where to store our config data in EEPROM
#define CONFIG_START 0

template <class T>
class Settings {
  // This is for mere detection if they are our settings
  char version[4];
public:
  T s;

  Settings() {
      // default settings
    version[0] = CONFIG_VERSION[0];
    version[1] = CONFIG_VERSION[1];
    version[2] = CONFIG_VERSION[2];
    version[3] = CONFIG_VERSION[3];
  }

  void load() {
    // To make sure there are settings, and they are OURS!
    // If nothing is found it won't change anything.

    if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2]) {
      for (unsigned int t=0; t<sizeof(T); t++) {
        *((char*)&s + t) = EEPROM.read(CONFIG_START + sizeof(version) - 1 + t);
      }
    }
  }

  void save() {
    for (unsigned int t=0; t<3; t++) {
      EEPROM.write(CONFIG_START + t, CONFIG_VERSION[t]);
    }
      
    for (unsigned int t=0; t<sizeof(T); t++) {
      EEPROM.write(CONFIG_START + 3 + t, *((char*)&s + t));
    }
  }
};

#endif