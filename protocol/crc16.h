#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>
#include <stddef.h>

//creation polynome 0x1021, init 0xFFF
   
//data addr isnt constant but its value is
uint16_t crc16_ccitt(const uint8_t *data, size_t n);

#endif
