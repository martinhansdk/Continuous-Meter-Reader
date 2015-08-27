#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>

uint32_t crc_update(uint32_t crc, uint8_t data);
uint32_t crc_array(uint8_t *s, int len);

#endif
