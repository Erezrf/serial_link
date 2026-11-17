/* CRC FUNCTIONALITY CHECK
 */
#include <stdio.h>
#include "protocol/crc16.h"

int main(void)
{
    const uint8_t text[] = "123456789";            // 9 characters + '\0'- ending of a str at the end 9*10 bytes (char is byte long)
    uint16_t crc = crc16_ccitt(text, 9);            
    printf("crc16(\"123456789\") = 0x%04X  (expected 0x29B1)\n", crc);
    return 0;
}
