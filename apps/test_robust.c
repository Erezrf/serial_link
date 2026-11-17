/*end 100,000 frames through a noisy line and count what gets through.
   The number that matters is "WRONG value accepted" - it must be 0. 
   */
#include <stdio.h>
#include <stdint.h>
#include "../hal/channel.h"
#include "../protocol/frame.h"
#include "../protocol/parser.h"

#define N_FRAMES 100000u

static float expected[N_FRAMES];   // what we sent, indexed by sequence number from 0 to 99,999

int main(void)
// chanel configuration 
{
    const channel_cfg_t cfg = {
        .bit_error_rate = 1e-3,    // 1 bit in 1000 flips   
        .drop_rate      = 0.005,   // 0.5% of bytes lost    
        .garbage_rate   = 0.002,   // 0.2% random bytes added 
        .seed           = 12345u,
    };
    channel_t ch;
    //initializations of channel and parser
    channel_init(&ch, &cfg);

    parser_t p;
    parser_init(&p);

// counters settings, the frames and number of bytes, and f an empty srtruct that will hold the parsed data
    uint32_t rng = 2463534242u;     
    uint32_t correct = 0, wrong = 0;
    uint8_t  frame[FRAME_MAX_LEN], line_out[2];
    frame_t  f;

    //encoding a fp number by a random val selected
    for (uint32_t seq = 0; seq < N_FRAMES; seq++) {
        rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5;
        float value   = ((float)(rng % 2000001u) - 1000000.0f) / 1000.0f;  /* -1000.000 .. 1000.000 */
        expected[seq] = value;

        float vals[2] = { (float)seq, value };
        size_t len = frame_encode(MSG_TWO_F32, vals, 2, frame);

        //sending the byte into the noisy channel and parsing it afterwards
        for (size_t i = 0; i < len; i++) {
            size_t k = channel_pass(&ch, frame[i], line_out);
            for (size_t j = 0; j < k; j++) {
                if (!parser_feed(&p, line_out[j], &f)) {
                    continue;
                }
                // a frame passed the check and checking if this is the real value which has been sent
                float s = (f.n == 2) ? f.v[0] : -1.0f;
                if (f.id == MSG_TWO_F32 && s >= 0.0f && s < (float)N_FRAMES &&
                    s == (float)(uint32_t)s && f.v[1] == expected[(uint32_t)s]) {
                    correct++;
                } else {
                    wrong++;
                }
            }
        }
    }

    printf("check type            : %s\n",

        //when the test run on regular weak xor oparations
#ifdef CHECK_XOR
           "8-bit XOR"
#else
           "CRC-16-CCITT"
#endif
    );
    printf("frames sent           : %u\n", N_FRAMES);
    printf("line damage           : %u bits flipped, %u bytes dropped, %u bytes inserted\n",
           ch.bits_flipped, ch.bytes_dropped, ch.bytes_inserted);
    printf("received correctly    : %u (%.2f%%)\n", correct, 100.0 * correct / N_FRAMES);
    printf("rejected by check     : %u\n", p.check_err);
    printf("rejected by ID / LEN  : %u / %u\n", p.id_err, p.len_err);
    printf("lost (never decoded)  : %u\n", N_FRAMES - correct);
    printf("WRONG value accepted  : %u   <-- must be 0\n", wrong);
    return wrong ? 1 : 0;
}
