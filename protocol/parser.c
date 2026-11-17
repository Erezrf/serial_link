#include "parser.h"
#include <string.h>

void parser_init(parser_t *p)
{
    memset(p, 0, sizeof *p);
    p->state = WAIT_SYNC;
}

//static func, if the err noise is sync chars by accident, we start listenning for new framews, else, we continue to next GET_ID state 
static void restart(parser_t *p, uint8_t b)
{
    p->state = (b == FRAME_SYNC) ? GET_ID : WAIT_SYNC;
}

// parser feed func which returns TRUE only when a full valid frame parsed
bool parser_feed(parser_t *p, uint8_t b, frame_t *out)
{
    switch (p->state) {
        // continous only if the right char recieved
    case WAIT_SYNC:
        if (b == FRAME_SYNC) {
            p->state = GET_ID;
        }
        break;                                  

    case GET_ID:
        if (frame_payload_len(b) == 0u) {       // if payload len is incorrect, returns to beginning
            p->id_err++; //adding to err counter
            restart(p, b);
        } else {
            p->buf[0] = b; 
            p->state  = GET_LEN;
        }
        break;

    case GET_LEN:
        if (b != frame_payload_len(p->buf[0])) {   //length must match the id 
            p->len_err++;
            restart(p, b);
        } else {
            p->buf[1] = b;
            p->len    = b;
            p->idx    = 0;
            p->state  = GET_PAYLOAD;
        }
        break;

    case GET_PAYLOAD:
        p->buf[2 + p->idx] = b; // 2 first bytes occupied by id and len
        p->idx++;
        if (p->idx == p->len) {      // only steps forward when payload num collected equals the len expected
            p->state = GET_CHECK_LO;
        }
        break;

    case GET_CHECK_LO:             
        p->check_rx = b;
        p->state    = GET_CHECK_HI;
        break;

    case GET_CHECK_HI:
        p->check_rx |= (uint16_t)((uint16_t)b << 8); // unite both LO HI crc val named as check_rx
        p->state     = WAIT_SYNC;               // restarting the states

        if (frame_check(p->buf, 2u + p->len) != p->check_rx) {  //validate the crc and len of the recieved frame (payload only without sync)
            p->check_err++;
            return false;
        }

        // resolving id, num of fp delivered and assemling the numbers back again as fp vals
        out->id = p->buf[0];                
        out->n  = (uint8_t)(p->len / 4u);
        for (uint8_t k = 0; k < out->n; k++) {
            out->v[k] = get_f32_le(&p->buf[2 + 4u * k]);
        }
        p->ok++;         //correct frames counter up
        return true;
    }
    return false;
}
