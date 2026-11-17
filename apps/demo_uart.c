#include <stdio.h>
#include "../hal/uart_sim.h"
#include "../hal/sim_time.h"
#include "../drivers/ring_buffer.h"
#include "../protocol/frame.h"
#include "../protocol/parser.h"

#define N_FRAMES  1000u          // sensor transmit 1000 frames 
#define WORK_US   200u          // the main CPU has 200us of work to do other than communication with the sensor > Bytes transmitted by sensor

static ring_buffer_t rx_rb;    // defining ring buffer 

// pushing byte into the ring buffer when its ready
static void uart_rx_isr(uint8_t b)
{
    rb_push(&rx_rb, b);
}

//simulating the occupation of the CPU, looping for 200us
static void do_work_chunk(void)
{
    wait_until_ns(now_ns() + WORK_US * 1000ull);   /* stands for useful computation */
}

//definning 3 data exceptance strategies
typedef enum { BLOCKING_POLL, POLL_BETWEEN_WORK, INTERRUPT_RING_BUFFER } strategy_t;
static const char *names[] = { "polling (blocking)", "polling between work", "interrupt + ring buffer" };

static void run(strategy_t s)
{
    //initialization of the channel and the strategy selected
    const channel_cfg_t clean = { 0.0, 0.0, 0.0, 1u };
    uart_init(&clean, UART_BYTE_TIME_US_115200,
              s == INTERRUPT_RING_BUFFER ? UART_MODE_INTERRUPT : UART_MODE_POLLING,
              uart_rx_isr);
    rb_init(&rx_rb); //ring buffer init

    uint8_t frame[FRAME_MAX_LEN]; //loading frames to channel as the protocol runs for fp
    for (uint32_t i = 0; i < N_FRAMES; i++) {
        float vals[2] = { (float)i, 0.5f * (float)i };
        uart_send(frame, frame_encode(MSG_TWO_F32, vals, 2, frame)); //sending the encoded value to the wire
    }

    //parser initialization
    parser_t p;
    parser_init(&p);
    frame_t  f;
    uint32_t frames = 0, work = 0;
    uint8_t  b;

    uint64_t t0 = now_ns();
    uart_start();

    switch (s) {
    case BLOCKING_POLL:                 //main does nothing but wait for bytes 
        for (;;) {
            if (uart_rx_ready()) {
                if (parser_feed(&p, uart_rx_read(), &f)) frames++; //pushing bytes to parser
            } else if (uart_done() && !uart_rx_ready()) { //if not ready or done, looping for waiting
                break;
            }
        }
        break;

    case POLL_BETWEEN_WORK:             // check the flag between work chunks 
        for (;;) {
            do_work_chunk(); work++; //after finnishing, checking the flag
            if (uart_rx_ready()) {
                if (parser_feed(&p, uart_rx_read(), &f)) frames++;
            } else if (uart_done()) {
                break;
            }
        }
        break;

    case INTERRUPT_RING_BUFFER:         //ISR fills the buffer, main drains it 
        for (;;) {
            do_work_chunk(); work++;
            while (rb_pop(&rx_rb, &b)) { // 
                if (parser_feed(&p, b, &f)) frames++;
            }
            if (uart_done() && rb_count(&rx_rb) == 0) break; //ring buffer empty and uart done transmitting
        }
        break;
    }

    uart_join();
    double secs = (double)(now_ns() - t0) / 1e9;
    double busy = (double)work * WORK_US / 1e6;
    printf("%-26s %8u/%u %10u %10u %10u %9.0f%%\n", names[s], frames, N_FRAMES,
           uart_overruns(), atomic_load(&rx_rb.overflows), work, 100.0 * busy / secs);
}

int main(void)
{
    printf("%u frames x 13 bytes at 115200 baud, work chunk = %u us\n\n", N_FRAMES, WORK_US);
    printf("%-26s %13s %10s %10s %10s %10s\n", "strategy", "frames ok", "overruns", "rb full", "work done", "CPU free");
    run(BLOCKING_POLL);
    run(POLL_BETWEEN_WORK);
    run(INTERRUPT_RING_BUFFER);
    return 0;
}
