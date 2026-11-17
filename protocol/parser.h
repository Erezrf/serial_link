#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include "frame.h"

// all states names 
typedef enum {
    WAIT_SYNC,       // 0xAA                  
    GET_ID,          // next byte is MSG_ID               
    GET_LEN,         // next byte is LEN                  
    GET_PAYLOAD,     // collecting LEN payload bytes      
    GET_CHECK_LO,    // low byte of crc            
    GET_CHECK_HI     // high byte of crc            
} pstate_t;

// the memory structure of the reciever, stores the curr state and more details
typedef struct {
    pstate_t state;
    uint8_t  buf[2 + FRAME_MAX_PAYLOAD];  // the temp arr of the recieved frame in build process
    uint8_t  len;                         // expected payload len        
    uint8_t  idx;                         // payload bytes collected so far  
    uint16_t check_rx;                    // check value received of crc          
    // statistics counters
    uint32_t ok, check_err, id_err, len_err;
} parser_t;

// the complete parsed translated frame
typedef struct {
    uint8_t id;
    uint8_t n;                            // number of floats  
    float   v[FRAME_MAX_PAYLOAD / 4];     // the fp values
} frame_t;

//init function for parsing process
void parser_init(parser_t *p); 

// returns TRUE only when the parsing process done, state machine promotion func
bool parser_feed(parser_t *p, uint8_t b, frame_t *out);

#endif
