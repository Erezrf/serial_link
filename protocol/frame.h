#ifndef FRAME_H
#define FRAME_H

#include <stdint.h>
#include <stddef.h>

/*  floating points template:
 
   | SYNC | MSG_ID | LEN | PAYLOAD (LEN bytes) | CHECK_LO | CHECK_HI |
     0xAA   1 byte  1 byte  4 bytes per float      CRC field
 */
#define FRAME_SYNC         0xAAu     //  sync chars unsigned val
#define MSG_ONE_F32        0x01u     //  1 float  (4 bytes) 
#define MSG_TWO_F32        0x02u     //  2 floats (8 bytes) 

#define FRAME_MAX_PAYLOAD  8u        // when 2 fp sent
#define FRAME_HEADER_LEN   3u        // SYNC + ID + LEN = total size of headers in fp frame
#define FRAME_CHECK_LEN    2u        // CRC total len in fp frame
#define FRAME_MAX_LEN      (FRAME_HEADER_LEN + FRAME_MAX_PAYLOAD + FRAME_CHECK_LEN) // max len of fp frame = 13 bytes

// recieves id, returns the expected len in bytes, for unknown ID, returns 0
uint8_t  frame_payload_len(uint8_t msg_id);

// integrity check over the frame,and if built with -DCHECK_XOR- 8-bit XOR
uint16_t frame_check(const uint8_t *data, size_t n);

// put float v in a continues addresses in mem, in LITTLE ENDIAN method in dst address
void  put_f32_le(uint8_t *dst, float v);

// reciever side, recreat the fp  
float get_f32_le(const uint8_t *src);

// build a frame ,returns the frame length, or 0 if n does not match msg_id
size_t frame_encode(uint8_t msg_id, const float *vals, uint8_t n, uint8_t *out);

#endif
