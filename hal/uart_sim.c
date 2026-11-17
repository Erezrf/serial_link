#include "uart_sim.h"
#include "sim_time.h"
#include <pthread.h>
#include <stdatomic.h>

#define WIRE_MAX (1u << 20) //1 MB

static uint8_t      wire[WIRE_MAX];       //the actual data that will be sent from UART sim, 1 MB long
static size_t       wire_len;             // curr num of bytes on the wire
static channel_t    ch;
static uint64_t     byte_time_ns;
static uart_mode_t  mode;
static void       (*isr)(uint8_t);

//identifier of the thread of the bits on the wire
static pthread_t    line;

//the "hardware registers" 
static volatile uint8_t RXBUF;            //"REG" which collects the bits
// atomic usage because both threads will reach those vars
static _Atomic bool     rx_flag;          /* RXIFG: data waiting   */
static _Atomic uint32_t overruns;
static _Atomic bool     done;

//the UART init func
void uart_init(const channel_cfg_t *cfg, unsigned byte_time_us,
               uart_mode_t m, void (*rx_isr)(uint8_t))
{
    channel_init(&ch, cfg);
    byte_time_ns = (uint64_t)byte_time_us * 1000u;  //ns conversion
    mode     = m;
    isr      = rx_isr; //isr rx pointer
    wire_len = 0;
    //updating the registers due to the curr status
    atomic_store(&rx_flag, false);
    atomic_store(&overruns, 0);
    atomic_store(&done, false);
}

//copying n Bytes ftom data recives to the wire
void uart_send(const uint8_t *data, size_t n)
{
    for (size_t i = 0; i < n && wire_len < WIRE_MAX; i++) {
        wire[wire_len++] = data[i];
    }
}

//moment of delivering data to the "hardware" when Byte fully arrived
static void deliver(uint8_t b)
{
    // interrupt mode- calling isr by the byte
    if (mode == UART_MODE_INTERRUPT) {
        isr(b);                                   
    } else {
    // polling mode- if flag risen, overrun occurs, buffer=Byte by overwritten
        if (atomic_load(&rx_flag)) {              
            atomic_fetch_add(&overruns, 1);
        }
        RXBUF = b;                                
        atomic_store(&rx_flag, true);
    }
}

//thread which runs parallel to main thread
static void *line_thread(void *arg)
{
    (void)arg;
    uint64_t t = now_ns(); //having the exact start time of delivery
    uint8_t  out[2]; //defining out as 2 Bytes size

    // calling noisy channel delivery with channel pass dunc
    for (size_t i = 0; i < wire_len; i++) {
        size_t k = channel_pass(&ch, wire[i], out); // calling noisy channel delivery with channel pass dunc, having k num of succeeded Bytes
        for (size_t j = 0; j < k; j++) { //k represent the number of Bytes which sent on the wire under the noise
            t += byte_time_ns;                    
            wait_until_ns(t); //expected time to the Byte to arrive, thread stopps
            deliver(out[j]); //takes the correct Byte and deliver it to the reciever
        }
    }
    //transmision ended
    atomic_store(&done, true);
    return NULL;
}

void uart_start(void)    { pthread_create(&line, NULL, line_thread, NULL); }
void uart_join(void)     { pthread_join(line, NULL); }
bool uart_done(void)     { return atomic_load(&done); }

//flags reading
bool uart_rx_ready(void) { return atomic_load(&rx_flag); }

//reading the register clears the flag 
uint8_t uart_rx_read(void)
{
    uint8_t b = RXBUF;
    atomic_store(&rx_flag, false);               
    return b;
}

uint32_t uart_overruns(void) { return atomic_load(&overruns); }
