#include "channel.h"

// random number generator by xoring and shifting
static uint32_t next_rand(channel_t *ch)
{
    uint32_t x = ch->rng;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    ch->rng = x;
    return x;
}

static double rand01(channel_t *ch)            // normalization of the next rand num which been generated to [0,1]
{
    return (double)next_rand(ch) / 4294967296.0;
}

void channel_init(channel_t *ch, const channel_cfg_t *cfg)
{
    ch->cfg = *cfg;
    // seed= rng first number for the simulation as a random number
    ch->rng = cfg->seed ? cfg->seed : 1u;       //defining cfg as seed inserted or 1 if its 0 by default, under the cond of user entered seed!=0
    ch->bits_flipped = ch->bytes_dropped = ch->bytes_inserted = 0; //zeroing the statistical counters
}

// mixture between bugs inserted in out signal and in signals according to probability values for each bug
// input comes byte by byte and output 2 bytes
size_t channel_pass(channel_t *ch, uint8_t in, uint8_t out[2])
{
    // the pointer to out arr
    size_t n = 0;

    if (rand01(ch) < ch->cfg.drop_rate) {       // byte lost on the line, manipulating output signal
        ch->bytes_dropped++;
        return 0;
    }
    if (rand01(ch) < ch->cfg.garbage_rate) {    // noise byte appears before it, manipulating output signal
        out[n++] = (uint8_t)next_rand(ch);
        ch->bytes_inserted++;
    }
    for (int bit = 0; bit < 8; bit++) {         // each bit may flip independently, manipulating input signal
        if (rand01(ch) < ch->cfg.bit_error_rate) {
            in ^= (uint8_t)(1u << bit);
            ch->bits_flipped++;
        }
    }
    out[n++] = in;
    return n;
}
