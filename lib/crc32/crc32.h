#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t crc_update(uint32_t crc, uint8_t data);
uint32_t crc_array(uint8_t *s, int len);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
