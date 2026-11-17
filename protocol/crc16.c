#include "crc16.h"

uint16_t crc16_ccitt(const uint8_t *data, size_t n) // considering only the addr of the data sent
{
    uint16_t crc = 0xFFFF;                       // 0xFFFF value for xoring the data arrives 16 bit long

    for (size_t i = 0; i < n; i++) {
        crc ^= (uint16_t)((uint16_t)data[i] << 8);   /* data[i] is a byte long, shl of it = 16 bit long, xoring with CRC 
                                                        used for padding by 0s each byte- for nested loop and first xor operation */
        for (int bit = 0; bit < 8; bit++) {
            if (crc & 0x8000u) {                     
                crc = (uint16_t)((crc << 1) ^ 0x1021u);  // xoring result with the creation polynome
            } else {
                crc = (uint16_t)(crc << 1);              // if msb is 0 moving on to the next bit
            }
        }
    }
    return crc;
}
