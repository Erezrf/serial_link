// checking functionality of the ring buffer 
#include <stdio.h>
#include <pthread.h> //c library which can make another process to run simultanuesly
#include "../drivers/ring_buffer.h"

#define STRESS_BYTES 5000000u //number of bytes which will be sent when stress
 
//unit test log template
static int failures = 0;
#define CHECK(cond, msg) do { \
    if (cond) { printf("  PASS  %s\n", msg); } \
    else      { printf("  FAIL  %s  (line %d)\n", msg, __LINE__); failures++; } \
} while (0)

//global static variable for both processes
static ring_buffer_t rb;

//producer imitation
static void *producer(void *arg)
{
    (void)arg;
    //loop runs in stress mode loop
    for (uint32_t i = 0; i < STRESS_BYTES; i++) {
        while (!rb_push(&rb, (uint8_t)i)) {
            //we wait for the buffer to have enough space for another byte and only then its available to have more data into it
        }
    }
    return NULL;
}

int main(void)
{
    uint8_t b;

    printf("[basic]\n");
    // basic functionality check
    rb_init(&rb); //initialization of a new buffer
    CHECK(!rb_pop(&rb, &b), "pop from empty buffer fails"); //trying to pop from it
    CHECK(rb_push(&rb, 0x11) && rb_push(&rb, 0x22), "push two bytes"); //pushing into it
    CHECK(rb_count(&rb) == 2, "count == 2"); //checking if size log correct
    // checking FIFO functionality
    CHECK(rb_pop(&rb, &b) && b == 0x11, "first out is first in (FIFO)"); 
    CHECK(rb_pop(&rb, &b) && b == 0x22, "second out is second in");

    //another initialization and ov check
    rb_init(&rb);
    int pushed = 0;
    while (rb_push(&rb, 0x55)) pushed++;
    CHECK(pushed == (int)RB_SIZE - 1, "holds RB_SIZE-1 bytes (one slot kept empty)");
    CHECK(atomic_load(&rb.overflows) == 1, "push when full counts an overflow");

    //concurrent stress check
    printf("[stress: 2 threads, %u bytes]\n", STRESS_BYTES);
    rb_init(&rb);
    pthread_t t;
    pthread_create(&t, NULL, producer, NULL); //aniother thread runs except the main one, t and main

    uint32_t received = 0, out_of_order = 0;
    while (received < STRESS_BYTES) {
        if (rb_pop(&rb, &b)) {
            if (b != (uint8_t)received) out_of_order++;
            received++;
        }
    }
    pthread_join(t, NULL); //waiting for the producer thread to be end
    CHECK(received == STRESS_BYTES, "every byte arrived");
    CHECK(out_of_order == 0, "every byte arrived in order");

    printf("\n%s (%d failure%s)\n", failures ? "SOME TESTS FAILED" : "ALL TESTS PASSED",
           failures, failures == 1 ? "" : "s");
    return failures ? 1 : 0;
}
