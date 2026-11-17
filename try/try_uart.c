// UART check
#include <stdio.h>
#include "hal/uart_sim.h"

static void my_isr(uint8_t b)
{
    printf("ISR got byte 0x%02X\n", b);   //ISR prints
}

int main(void)
{
    const channel_cfg_t clean = { 0.0, 0.0, 0.0, 1u }; //channel definitions
    const uint8_t msg[] = { 0x48, 0x49, 0x21 };   //the Bytes on the channel

    //interrupt mode
    uart_init(&clean, UART_BYTE_TIME_US_115200, UART_MODE_INTERRUPT, my_isr);
    uart_send(msg, sizeof msg);
    uart_start();
    uart_join();

    //polling mode
    uart_init(&clean, UART_BYTE_TIME_US_115200, UART_MODE_POLLING, NULL);
    uart_send(msg, sizeof msg);
    uart_start();
    int got = 0;
    while (got < 3) {
        if (uart_rx_ready()) {
            printf("polled byte 0x%02X\n", uart_rx_read());
            got++;
        }
    }
    uart_join();
    printf("overruns: %u\n", uart_overruns());
    return 0;
}
