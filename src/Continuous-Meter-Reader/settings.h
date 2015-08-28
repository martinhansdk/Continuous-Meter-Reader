#ifndef SETTINGS_H
#define SETTINGS_H

#include <EEPROM.h>


// ID of the settings block
#define CONFIG_VERSION "met"

// Tell it where to store your config data in EEPROM
#define CONFIG_START 0

// Example settings structure
class Settings {
  // This is for mere detection if they are our settings
  char version[4];
public:
  // The variables of our settings
  int32_t meterId ;
  uint32_t seriesId;

  Settings();
  void load();
  void save();
};

#endif