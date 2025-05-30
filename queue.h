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
    
    #define MAX_QUEUE_SIZE 20 // The maximum amount of transmissions that can be queued
    #define MAX_DATA_SIZE 146 // The maximum amount of data per transmission
    
    // structure of elements in the queue
    typedef struct {
        volatile uint8_t address_RW; // address + R/nW bit
        volatile uint8_t data[MAX_DATA_SIZE];      // the data to be sent, or a null ptr if reading
        volatile unsigned int data_size;      // the number of bytes to be written or read
        volatile unsigned int read_bytes;
    } Transmission;
    
    /* Add a transmission to the queue.
     * 
     * @param element   Transmission to add to add to the queue.
     */
    uint8_t enqueue(uint8_t element);
    
    /* Remove the next transmission from the queue.
     * 
     * @returns     The next transmission in the queue, or null if the queue is empty.
     */
    uint8_t dequeue();
    
    /* Get the number of transmissions in the queue.
     * 
     * @returns     The number of transmissions in the queue
     */
    uint8_t getQueueSize();
    
    /*  Get the next transmission from the queue without removing it.
     * 
     * @returns     The next transmission in the queue, or null if the queue is empty.
     */
    uint8_t peek();


#ifdef	__cplusplus
}
#endif

#endif	/* PATER085_QUEUE_H */

