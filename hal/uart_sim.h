#ifndef UART_SIM_H
#define UART_SIM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "channel.h"

// A simulated UART receiver
typedef enum { UART_MODE_POLLING, UART_MODE_INTERRUPT } uart_mode_t; //enum with 2 modes- pooling and int

#define UART_BYTE_TIME_US_115200  87u   // 115200 baud

// UART init with channel properties cfg, selected mode, 
void     uart_init(const channel_cfg_t *cfg, unsigned byte_time_us,
                   uart_mode_t mode, void (*rx_isr)(uint8_t));
// UART send bytes on the wire rn
void     uart_send(const uint8_t *data, size_t n);  
// UART start thr transmission
void     uart_start(void);                         
// UART done status, weather all bytes arrived successfully
bool     uart_done(void);                           
// UART join for the message to arrive 
void     uart_join(void);                          

// vals which used in polling mode 
bool     uart_rx_ready(void);                       // RX flag                             
uint8_t  uart_rx_read(void);                        // read RX register, clears the flag     
uint32_t uart_overruns(void);

#endif
