#include <stdio.h>
#include <string.h>
#include "../protocol/crc16.h"
#include "../protocol/frame.h"
#include "../protocol/parser.h"

static int failures = 0;

// macro function for implementing a condition with a print respectively
//  for preprocessor to nex line as well
// printing the exacrt line which holds the failor 
#define CHECK(cond, msg) do { \
    if (cond) { printf("  PASS  %s\n", msg); } \
    else      { printf("  FAIL  %s  (line %d)\n", msg, __LINE__); failures++; } \
} while (0)

// simulating a comunication line, sending to parser feed and saves the last recieved frame as last
static int feed_all(parser_t *p, const uint8_t *data, size_t n, frame_t *last)
{
    int frames = 0;
    for (size_t i = 0; i < n; i++) {
        if (parser_feed(p, data[i], last)) {
            frames++;
        }
    }
    return frames;
}

//main test
int main(void)
{
    // same test as the try, equality check between the expected val of the crc of 1-9 num
    printf("[CRC]\n");
    const uint8_t check_str[] = "123456789";
    CHECK(crc16_ccitt(check_str, 9) == 0x29B1, "crc16(\"123456789\") == 0x29B1");

    // check if put and get doing their job well by store and get
    printf("[float bytes]\n");
    uint8_t b4[4];
    put_f32_le(b4, 1.0f);                               // 1.0f = 0x3F800000 
    CHECK(b4[0] == 0x00 && b4[1] == 0x00 && b4[2] == 0x80 && b4[3] == 0x3F, //LE stored
          "1.0f -> 00 00 80 3F (little-endian)");
    CHECK(get_f32_le(b4) == 1.0f, "get_f32_le(put_f32_le(1.0f)) == 1.0f");

    // checking the framing of 1 and 2 fp numbers
    printf("[encode]\n");
    uint8_t buf[FRAME_MAX_LEN];
    float one = 3.14f;
    size_t len1 = frame_encode(MSG_ONE_F32, &one, 1, buf);
    CHECK(len1 == 9, "one float -> 9 bytes (3 header + 4 + 2)");
    printf("        frame: ");
    for (size_t i = 0; i < len1; i++) printf("%02X ", buf[i]);
    printf("\n");
    float two[2] = { -12.5f, 1000.25f };
    CHECK(frame_encode(MSG_TWO_F32, two, 2, buf) == 13, "two floats -> 13 bytes");
    CHECK(frame_encode(MSG_ONE_F32, two, 2, buf) == 0, "wrong count for id -> 0");

    printf("[parser]\n");
    parser_t p;
    frame_t  f;

    // checking the whole sys
    parser_init(&p);
    size_t n = frame_encode(MSG_TWO_F32, two, 2, buf);
    CHECK(feed_all(&p, buf, n, &f) == 1 && f.n == 2 && f.v[0] == -12.5f && f.v[1] == 1000.25f,
          "clean frame decodes to the same two floats");

    // putting garbage before the frame
    parser_init(&p);
    uint8_t stream[64];
    size_t s = 0;
    const uint8_t junk[] = { 0x13, 0x37, 0xAA, 0x99, 0x00 };  // includes a fake SYNC 
    memcpy(stream, junk, sizeof junk); s += sizeof junk;
    n = frame_encode(MSG_ONE_F32, &one, 1, &stream[s]); s += n;
    CHECK(feed_all(&p, stream, s, &f) == 1 && f.v[0] == 3.14f, "garbage before frame is skipped");

    // one flipped bit gets rejected 
    parser_init(&p);
    n = frame_encode(MSG_ONE_F32, &one, 1, buf);
    buf[5] ^= 0x04;
    CHECK(feed_all(&p, buf, n, &f) == 0 && p.check_err == 1, "flipped bit -> check error, no frame");

    // bad frame followed by good frame, parser recovers
    parser_init(&p);
    s = 0;
    n = frame_encode(MSG_ONE_F32, &one, 1, &stream[s]); stream[s + 4] ^= 0xFF; s += n;
    n = frame_encode(MSG_ONE_F32, &one, 1, &stream[s]); s += n;
    CHECK(feed_all(&p, stream, s, &f) == 1 && f.v[0] == 3.14f, "recovers after a corrupted frame");

    //wrong LEN for the id gets rejected early 
    parser_init(&p);
    const uint8_t bad_len[] = { 0xAA, MSG_ONE_F32, 8 };
    feed_all(&p, bad_len, sizeof bad_len, &f);
    CHECK(p.len_err == 1 && p.state == WAIT_SYNC, "LEN that does not match ID -> len error");

    printf("\n%s (%d failure%s)\n", failures ? "SOME TESTS FAILED" : "ALL TESTS PASSED",
           failures, failures == 1 ? "" : "s");
    return failures ? 1 : 0;
}
