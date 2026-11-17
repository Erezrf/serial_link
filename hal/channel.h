#ifndef CHANNEL_H
#define CHANNEL_H

#include <stdint.h>
#include <stddef.h>

// model of a noisy serial line, cfg for configuration
typedef struct {
    double   bit_error_rate;   // probability that each single bit flips, for example 1 of 1000: 1e-3  
    double   drop_rate;        // probability that a byte is lost                     
    double   garbage_rate;     // probability that a random byte is inserted          
    uint32_t seed;             // same seed = same run, so results are reproducible and bugs are debugable  
} channel_cfg_t;

// the accumulated status of the channel
typedef struct {
    channel_cfg_t cfg; //the current settings of the channel that we selected to use
    uint32_t rng; // random number generator value for bugs 
    uint32_t bits_flipped, bytes_dropped, bytes_inserted; // bugs counters
} channel_t;

void   channel_init(channel_t *ch, const channel_cfg_t *cfg); //settings of the initialization func of the channel, ch and cfg set

// 0- byte dropped, 1- byte succeeded, 2- noised byte inserted
size_t channel_pass(channel_t *ch, uint8_t in, uint8_t out[2]);

#endif
