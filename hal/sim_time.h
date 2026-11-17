#ifndef SIM_TIME_H
#define SIM_TIME_H

#include <stdint.h>
#include <time.h>

// timeing functionality for UART transmission 
static inline uint64_t now_ns(void) //inline usage for optimizing static func call
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts); //monotonous timer counter, curr time
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

// waiting function for the baud rate delay now_ts function to reach the goal t value for right BR
static inline void wait_until_ns(uint64_t t)
{
    while (now_ns() < t) {
        /* spin */
    }
}

#endif

