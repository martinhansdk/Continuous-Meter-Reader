// minimal definitions to allow code to include avr/pgmspace.h when running unit tests on host

#ifndef __PGMSPACE_H
#define __PGMSPACE_H

#define PROGMEM
typedef uint32_t prog_uint32_t;

/** pgm_read_dword_near(address_short)
    Read a double word from the program space with a 16-bit (near) address. 
    \note The address is a byte address. 
    The address is in the program space. */

uint32_t pgm_read_dword_near(const uint32_t* address_short) {
    return *address_short;
}


#endif