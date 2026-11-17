// check of the channel interface
#include <stdio.h>
#include "hal/channel.h"

int main(void)
{
    // set a high risk channel 
    const channel_cfg_t cfg = { 0.05, 0.10, 0.10, 11u };
    channel_t ch;
    channel_init(&ch, &cfg);

    uint8_t out[2];
    printf("in  -> out\n");
    for (unsigned i = 0; i < 20; i++) {
        uint8_t in = (uint8_t)(0x10 + i);
        size_t k = channel_pass(&ch, in, out);

        printf("%02X  -> ", in);
        if (k == 0) printf("(dropped)");
        for (size_t j = 0; j < k; j++) printf("%02X ", out[j]);
        printf("\n");
    }
    printf("flipped bits=%u dropped=%u inserted=%u\n",
           ch.bits_flipped, ch.bytes_dropped, ch.bytes_inserted);
    return 0;
}
