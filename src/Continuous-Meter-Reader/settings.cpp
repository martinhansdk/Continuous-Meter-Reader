
#include "settings.h"

Settings::Settings() {
	// default settings
	version[0] = CONFIG_VERSION[0];
	version[1] = CONFIG_VERSION[1];
	version[2] = CONFIG_VERSION[2];
	version[3] = CONFIG_VERSION[3];
	meterId = 1;
	seriesId = 0;
}


void Settings::load() {
    // To make sure there are settings, and they are OURS!
    // If nothing is found it won't change anything.
	if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
		EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
		EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2]) {
		for (unsigned int t=0; t<sizeof(Settings); t++) {
			*((char*)this + t) = EEPROM.read(CONFIG_START + t);
		}
	}
}

void Settings::save() {
	for (unsigned int t=0; t<sizeof(Settings); t++)
		EEPROM.write(CONFIG_START + t, *((char*)this + t));
}
