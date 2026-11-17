// frame encoding check
#include <stdio.h>
#include "protocol/frame.h"

int main(void)
{
    uint8_t frame[FRAME_MAX_LEN]; // 
    float value = 3.14f;

    size_t len = frame_encode(MSG_ONE_F32, &value, 1, frame); // calling the encoding function by the args provided

    printf("frame length: %zu\n", len);
    printf("frame bytes : ");
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", frame[i]);
    }
    printf("\n");
    printf("decoded back: %f\n", get_f32_le(&frame[3]));
    return 0;
}
