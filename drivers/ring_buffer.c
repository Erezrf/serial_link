#include "ring_buffer.h"

//buffer initialization' atomic used for zero values to be published to all cpus
void rb_init(ring_buffer_t *rb)
{
    atomic_store(&rb->head, 0);
    atomic_store(&rb->tail, 0);
    atomic_store(&rb->overflows, 0);
}

// tries to push byte into the ring buffer, if full dump the byte and update ov val
bool rb_push(ring_buffer_t *rb, uint8_t b)
{
    uint16_t head = atomic_load(&rb->head);
    uint16_t next = (uint16_t)((head + 1u) & RB_MASK);

    if (next == atomic_load(&rb->tail)) {   // full, next==tail!
        atomic_fetch_add(&rb->overflows, 1); //updating ov val
        return false;                       // byte dropped
    }
    rb->buf[head] = b;                      //inserting byte to buffer's head
    atomic_store(&rb->head, next);          //writing accomplish
    return true;
}

// checks weather there is a new byte that can be released from ring buffer
bool rb_pop(ring_buffer_t *rb, uint8_t *out)
{
    uint16_t tail = atomic_load(&rb->tail);

    if (tail == atomic_load(&rb->head)) {   //
        return false;
    }
    *out = rb->buf[tail];                   //reading data from tail of the buffer
    atomic_store(&rb->tail, (uint16_t)((tail + 1u) & RB_MASK));  //tail step forward
    return true;
}

//returning the byte number that are in the buffer currently
uint16_t rb_count(ring_buffer_t *rb)
{
    return (uint16_t)((atomic_load(&rb->head) - atomic_load(&rb->tail)) & RB_MASK);
}
