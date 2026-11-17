// parsing validity check
#include <stdio.h>
#include "protocol/frame.h"
#include "protocol/parser.h"

int main(void)
{
    uint8_t frame[FRAME_MAX_LEN];
    float values[2] = { 1.5f, -20.25f };
    size_t len = frame_encode(MSG_TWO_F32, values, 2, frame);

    parser_t parser;
    parser_init(&parser);
    frame_t result;

    // feed the bytes one at a time, exactly like they would arrive from a UART, byte after byte
    for (size_t i = 0; i < len; i++) {
        if (parser_feed(&parser, frame[i], &result)) {
            printf("frame complete after byte %zu: id=%u, n=%u, values: %f %f\n",
                   i + 1, result.id, result.n, result.v[0], result.v[1]);
        }
    }
    printf("ok=%u check_err=%u\n", parser.ok, parser.check_err);
    return 0;
}
