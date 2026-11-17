#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>

#define RB_SIZE 256u                        
#define RB_MASK (RB_SIZE - 1u)

typedef struct {
    //atomic def for in order exe and no dependencies
    volatile uint8_t   buf[RB_SIZE];        //buffer definition
    _Atomic uint16_t   head;                //head of ring buffer definition
    _Atomic uint16_t   tail;                //tail of ring buffer definition
    _Atomic uint32_t   overflows;           //counter of the number of bytes that has been lost due to fast sending
} ring_buffer_t;

void     rb_init (ring_buffer_t *rb);
bool     rb_push (ring_buffer_t *rb, uint8_t b);      
bool     rb_pop  (ring_buffer_t *rb, uint8_t *out);   
uint16_t rb_count(ring_buffer_t *rb);

#endif
