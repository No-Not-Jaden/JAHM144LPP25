/*
 * File:   I2CLib.c
 * Author: Jaden Paterson
 *
 * Created on April 2, 2025, 10:17 AM
 */


#include "xc.h"
#include "I2CLib.h"
#include "queue.h"      // To queue up I2C transmissions 
#include "stdio.h"      // For NULL

#define MAX_EVENTS 3 // maximum number of events that can be registered

// pool of transmission object that can be allocated for the queue
// If the queue is full, 2 extra transmissions are needed for the active
// transmission and an incoming transmission from transmit_packet()
Transmission transmission_pool[MAX_QUEUE_SIZE + 2]; 
uint8_t transmission_used[MAX_QUEUE_SIZE + 2] = {0};

// The active transmission that is being sent
volatile Transmission* activeTransmission = NULL;
// The current data byte that is being sent/received
volatile unsigned int curDataIndex = 0; 

// The current I2C transmission stage
enum TransmissionStage {
    NONE, ENABLING, WRITE_ADDRESS, DATA, DISABLING
};

enum TransmissionStage stage = NONE;

// a function that takes in the byte received, number of remaining bytes, and returns the number of additional bytes to be read
typedef unsigned int receiveEvent(uint8_t, int); 

unsigned long eventAddresses[MAX_EVENTS]; // addresses of event functions to be called
uint8_t i2cAddresses[MAX_EVENTS]; // I2C addresses to match the eventAddresses
int numEvents = 0; // The number of events registered

/* Register a function to be called when data is received from I2C.
 * The function should be in the form: unsigned int receiveEvent(uint8_t, int);
 * The first parameter is the byte received, and the second parameter is the
 * number of bytes remaining in the transmission. The function should return the
 * number of bytes to extend the packet by. If you have a set packet length,
 * always return 0.
 * 
 * @param i2cAddress        The address of the I2C device that this event will fire for.
 * @param functionAddress   The address of the function to be called.
 */
void register_event(uint8_t i2cAddress, unsigned long functionAddress) {
    if (numEvents < MAX_EVENTS) {
        i2cAddresses[numEvents] = i2cAddress;
        eventAddresses[numEvents++] = functionAddress;
    }
}

void initiateTransmission() {
    stage = ENABLING;
    I2C1CONbits.SEN = 1;
    
    
}

// Get a free Transmission slot
Transmission* allocate_transmission() {
    for (int i = 0; i < MAX_DATA_SIZE + 2; i++) {
        if (!transmission_used[i]) {
            transmission_used[i] = 1;
            return &transmission_pool[i];
        }
    }
    return NULL; // no space
}

// Free after use
void free_transmission(Transmission* t) {
    int index = t - transmission_pool;
    if (index >= 0 && index < MAX_DATA_SIZE + 2) {
        transmission_used[index] = 0;
    }
}


int loadNextTransmission() {
    if (getQueueSize() > 0) {
        if (activeTransmission != NULL) {
            free_transmission(activeTransmission);
        }
        activeTransmission = dequeue();
        curDataIndex = 0;
        return 1;
    }
    return 0;
}

/* Send or receive data through I2C
 * 
 * @param address_RW    The least significant bit is for read / not write, and 
 *                      the 7 most significant bits are for the address.
 * @param data[]        The data to be sent. Use 0 for this parameter if reading.
 * @param data_size     The size of the data to be sent or received.
 */
void transmit_packet(uint8_t address_RW, uint8_t data[], int data_size) {
    
    // create a struct pointer to add to the queue
    Transmission* transmissionPtr = allocate_transmission();
    transmissionPtr->address_RW = address_RW;
    if (address_RW | 0b0) {
        // transfer data over to be written
        for (int i = 0; i < data_size && i < MAX_DATA_SIZE; i++) {
            transmissionPtr->data[i] = data[i];
        }
    }
    if (data_size < MAX_DATA_SIZE) {
        transmissionPtr->data_size = data_size;
    } else {
        transmissionPtr->data_size = MAX_DATA_SIZE;
    }
    
    while (enqueue(transmissionPtr) == 0);
    
    if (stage == NONE) {
        initiateTransmission();
    }
    
}

// transmit the next data that is needed to be sent
void transmitNextData() {
    // make sure read is false
    if (activeTransmission->address_RW | 0b0) {
        I2C1TRN = activeTransmission->data[curDataIndex++];
    }
}

void __attribute__((__interrupt__,__auto_psv__)) _MI2C1Interrupt(void) {
    _MI2C1IF = 0; // clear interrupt
    I2C1CON;
    
    if (stage == ENABLING && I2C1STATbits.S) {
        // Start bit was just sent
        loadNextTransmission();
        // write address
        I2C1TRN = activeTransmission->address_RW;
        stage = WRITE_ADDRESS;
    } else if (stage == DATA && activeTransmission->address_RW & 0b1) {
        // reading data
        if (I2C1STATbits.RBF == 1) {
            // receive byte is full - data is ready to be read
            // generate master acknowledge
            I2C1CONbits.ACKDT = 0;
            I2C1CONbits.ACKEN = 1;

            curDataIndex++;

            // read from the I2CxRCV register
            uint8_t data = I2C1RCV;

            // call events
            uint8_t activeAddress = activeTransmission->address_RW >> 1;
            for (int i = 0; i < numEvents; i++) {
                if (i2cAddresses[i] == activeAddress) {
                    receiveEvent* f = (receiveEvent*) eventAddresses[i];
                    activeTransmission->data_size += f(data, activeTransmission->data_size - curDataIndex);
                }
            }

        } else if (activeTransmission->data_size == curDataIndex) {
            // no more data to receive
            stage = DISABLING;
            I2C1CONbits.PEN = 1;   // Initiate STOP condition
        } else {
            I2C1CONbits.RCEN = 1; // enable receive
        }
    } else if (stage == DISABLING && I2C1STATbits.P) {
        if (loadNextTransmission()) {
            // more data to be sent
            initiateTransmission();
        } else {
            stage = NONE;
        }
    } else if (_TRSTAT == 0) {
        if (_ACKSTAT == 0) {
            // send next transmission
            if (stage == WRITE_ADDRESS) {
                // check if it is reading
                if (activeTransmission->address_RW & 0b1) {
                    // activate restart bit
                    // Note: There is no repeated start for the BNO085
                    // START ? [Read address + 1] ? Read SHTP Packet ? STOP
                    //I2C1CONbits.RSEN = 1; 
                    I2C1CONbits.RCEN = 1; // enable receive
                } else {
                    transmitNextData();
                }
                stage = DATA;
            } else if (stage == DATA) {
                if (activeTransmission->data_size > 0) {
                    transmitNextData();
                }
            }
        } else {
            // not acknowledged
            stage = DISABLING;
            I2C1CONbits.PEN = 1;   // Initiate STOP condition
        }
    }
}

// Initialize the I2C1 peripheral with 100k Hz baudrate
void init_i2c() {
    
    // I2C initialization
    I2C1CON = 0;
    I2C1CONbits.SCLREL = 1; // Clock holding
    I2C1BRG = 0x9D;         // 100k Hz baudrate
    _MI2C1IF = 0;           // clear interrupt flag
    _MI2C1IE = 1;           // enable interrupt later
    _MI2C1IP = 6;           // higher interrupt priority
    I2C1CONbits.I2CEN = 1;  // enable
    
}

