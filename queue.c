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


Transmission* queue[MAX_QUEUE_SIZE];
int queue_size = 0;
int nextOpenIndex = 0;
int lastDequeuedIndex = -1;

// Add an element to the queue
// returns 1 if the element was added successfully
int enqueue(Transmission* element) {
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
Transmission* dequeue() {
    if (queue_size == 0) {
        // empty
        return NULL;
    }
    // increment the lastDequeuedIndex
    lastDequeuedIndex = (lastDequeuedIndex + 1) % MAX_QUEUE_SIZE;
    queue_size--;
    return queue[lastDequeuedIndex];
}

Transmission* peek() {
    if (queue_size == 0) {
        // empty
        return NULL;
    }
    int peekIndex = (lastDequeuedIndex + 1) % MAX_QUEUE_SIZE;
    return queue[peekIndex];
}

int getQueueSize() {
    return queue_size;
}

