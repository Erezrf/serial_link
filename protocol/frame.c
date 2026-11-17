#include "frame.h"
#include "crc16.h"
#include <string.h>

// called by sender and the reciever while building the frame or unpacking it
uint8_t frame_payload_len(uint8_t msg_id)
{
    switch (msg_id) {
    case MSG_ONE_F32: return 4u;
    case MSG_TWO_F32: return 8u;
    default:          return 0u;     // if the ID sent is unknown
    }
}

// returning crc value under the compilation settings defined by the user
uint16_t frame_check(const uint8_t *data, size_t n)
{
#ifdef CHECK_XOR
    uint8_t x = 0;                   // weak xor crc build
    for (size_t i = 0; i < n; i++) {
        x ^= data[i];
    }
    return x;
#else
    return crc16_ccitt(data, n); // regular build by crcr_ccitt function
#endif
}

// copying the number in order to store it as LE in mem
void put_f32_le(uint8_t *dst, float v)
{
    uint32_t u;
    memcpy(&u, &v, sizeof u);        // copying fp as hex in order to shift it for storage
    dst[0] = (uint8_t)(u);           // little-endian storing 
    dst[1] = (uint8_t)(u >> 8);
    dst[2] = (uint8_t)(u >> 16);
    dst[3] = (uint8_t)(u >> 24);
}

// building the fp number back by FIFO in order to implement BE back
float get_f32_le(const uint8_t *src)
{
    uint32_t u = (uint32_t)src[0]
               | ((uint32_t)src[1] << 8)
               | ((uint32_t)src[2] << 16)
               | ((uint32_t)src[3] << 24);
    float v;
    memcpy(&v, &u, sizeof v);
    return v;
}

// building the fp frame, arg out provided by calles, manipulating the address of it, and returning the bytes len of the frame
size_t frame_encode(uint8_t msg_id, const float *vals, uint8_t n, uint8_t *out)
// checking frame validity by its len using frame payload func
{
    uint8_t len = frame_payload_len(msg_id);
    if (len == 0u || len != (uint8_t)(4u * n)) {
        return 0;                    // id unknown or wrong number of values 
    }

    size_t i = 0;
    out[i++] = FRAME_SYNC; //placing sync val for first byte
    out[i++] = msg_id;     //placing msg_id val for first byte (arg of the func)
    out[i++] = len;        //placing len str val first byte
    for (uint8_t k = 0; k < n; k++) {
        put_f32_le(&out[i], vals[k]);  //sends next available address as an arg of put func, and the curr byte
        i += 4;
    }

    // sends the payload used for crc calc (without sync)
    uint16_t chk = frame_check(&out[1], i - 1);
    out[i++] = (uint8_t)(chk);           //first crc byte placing
    out[i++] = (uint8_t)(chk >> 8);      //second crc byte placing
    return i;
}
