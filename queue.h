/* 
 * File:   queue.h
 * Author: Jaden Paterson
 *
 * Created on March 14, 2025, 5:38 PM
 */

#include "stdint.h"

#ifndef PATER085_QUEUE_H
#define	PATER085_QUEUE_H

#ifdef	__cplusplus
extern "C" {
#endif
    
    #define MAX_QUEUE_SIZE 24
    #define MAX_DATA_SIZE 146
    
    // structure of elements in the queue
    typedef struct {
        uint8_t address_RW; // address + R/nW bit
        uint8_t data[MAX_DATA_SIZE];      // the data to be sent, or a null ptr if reading
        unsigned int data_size;      // the number of bytes to be written or read
        unsigned int read_bytes;
    } Transmission;
    
    int enqueue(Transmission*);
    Transmission* dequeue();
    int getQueueSize();
    Transmission* peek();


#ifdef	__cplusplus
}
#endif

#endif	/* PATER085_QUEUE_H */

