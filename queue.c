/*
 * File:   queue.c
 * Author: Jaden Paterson
 *
 * Created on March 14, 2025, 5:36 PM
 */


#include "xc.h"
#include "stdint.h"
#include "queue.h"
#include "stdio.h"


volatile uint8_t queue[MAX_QUEUE_SIZE];
volatile uint8_t queue_size = 0;
volatile uint8_t nextOpenIndex = 0;
volatile uint8_t lastDequeuedIndex = MAX_QUEUE_SIZE - 1;

// Add an element to the queue
// returns 1 if the element was added successfully
uint8_t enqueue(uint8_t element) {
    if (queue_size == MAX_QUEUE_SIZE) {
        // full
        return 0;
    }
    // add element
    queue[nextOpenIndex] = element;
    queue_size++;
    // set next open index
    nextOpenIndex = (nextOpenIndex + 1) % MAX_QUEUE_SIZE;
    return 1;
}



// removes the next element from the queue
uint8_t dequeue() {
    if (queue_size == 0) {
        // empty
        return 255;
    }
    // increment the lastDequeuedIndex
    lastDequeuedIndex = (lastDequeuedIndex + 1) % MAX_QUEUE_SIZE;
    queue_size--;
    return queue[lastDequeuedIndex];
}

uint8_t peek() {
    if (queue_size == 0) {
        // empty
        return 255;
    }
    int peekIndex = (lastDequeuedIndex + 1) % MAX_QUEUE_SIZE;
    return queue[peekIndex];
}

uint8_t getQueueSize() {
    return queue_size;
}

