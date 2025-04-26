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
volatile Transmission transmission_pool[MAX_QUEUE_SIZE + 2]; 
volatile uint8_t transmission_used[MAX_QUEUE_SIZE + 2] = {0};

// The active transmission that is being sent
volatile uint8_t activeTransmissionIndex = 255;
volatile Transmission activeTransmission;
// The current data byte that is being sent/received
volatile unsigned int curDataIndex = 0; 

// Whether I2C is initialized or not.
uint8_t initialized = 0;

// variables to keep track of any I2C interrupts when the interrupt is disabled.
volatile uint8_t I2CInterruptQueued = 0;
volatile uint8_t I2CInterruptEnabled = 1;

// The current I2C transmission stage
enum TransmissionStage {
    NONE, ENABLING, WRITE_ADDRESS, DATA, DISABLING
};

enum TransmissionStage stage = NONE;

// a function that takes in the byte received, number of remaining bytes, and returns the number of additional bytes to be read
typedef unsigned int receiveEvent(uint8_t, int); 

void handleMI2CInterrupt();

volatile unsigned long eventAddresses[MAX_EVENTS]; // addresses of event functions to be called
volatile uint8_t i2cAddresses[MAX_EVENTS]; // I2C addresses to match the eventAddresses
volatile int numEvents = 0; // The number of events registered

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

/* Initiate an I2C transmission by sending the start bit.
 */
void initiateTransmission() {
    stage = ENABLING;
    I2C1CONbits.SEN = 1;
}

/* Stop an I2C transmission by sending the stop bit.
 */
void stopTransmission() {
    stage = DISABLING;
    I2C1CONbits.PEN = 1;
}

/* Get the number of transmissions that have been allocated.
 * 
 * @returns     The number of transmissions currently being used,
 */
int getTransmissionsUsed() {
    int count = 0;
    for (int i = 0; i < MAX_QUEUE_SIZE + 2; i++) {
        if (transmission_used[i]) {
            count++;
        }
    }
    return count;
}

/* Allocate a transmission to use.
 * 
 * @returns     The index of a transmission in the transmission_pool that should be used.
 */
uint8_t allocate_transmission() {
    for (int i = 0; i < MAX_QUEUE_SIZE + 2; i++) {
        if (!transmission_used[i]) {
            transmission_used[i] = 1;
            return i;
        }
    }
    // no space, all transmissions used
    if (getQueueSize() == 0) {
        // queue size should be equal to or 1 less than allocated transmissions
        // this means that some transmissions were unable to be freed
        for (int i = 0; i < MAX_QUEUE_SIZE + 2; i++) {
            // manually free transmissions other than the active one
            if (transmission_used[i] == 1 && i != activeTransmissionIndex) {
                transmission_used[i] = 0;
            }
        }
        return allocate_transmission();
    }
    return 255; // no space
}

/* Free an allocated transmission, so it can be reused in the future.
 * 
 * @param index     Index of the allocated transmission in the transmission_pool.
 */
void free_transmission(uint8_t index) {
    if (index >= 0 && index < MAX_QUEUE_SIZE + 2) {
        transmission_used[index] = 0;
    }
}

/* Disable I2C relevant interrupts.
 */
void disableInterrupts() {
    I2CInterruptEnabled = 0;
    // these 2 used for the BNO085
    _T1IE = 0;
    _INT0IE = 0;
}

/* Check if I2C is ready to receive an interrupt.
 * 
 * @returns     True if the I2C logic is waiting for an interrupt.
 */
uint8_t isI2CReady() {
    return (I2C1CON & 0x1F) == 0x00 && _TRSTAT == 0 && stage != NONE;
}

/* Enable I2C relevant interrupts, and force the MI2C1 interrupt if one was missed.
 */
void enableInterrupts() {
    // these 2 are for the BNO085
    _T1IE = 1;
    _INT0IE = 1;
    
    I2CInterruptEnabled = 1;
    if (I2CInterruptQueued) {
        handleMI2CInterrupt();
    }
}

/* Load the next transmission to the activeTransmission variables.
 * 
 * @returns     True if a transmission was loaded.
 */
int loadNextTransmission() {
    if (getQueueSize() > 0) {
        disableInterrupts();
        activeTransmissionIndex = dequeue();
        activeTransmission = transmission_pool[activeTransmissionIndex];
        free_transmission(activeTransmissionIndex);
        enableInterrupts();
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
void transmit_packet(uint8_t address_RW, uint8_t data[], unsigned int data_size) {
    if (data_size > 0) {
        if (address_RW & 0b1) {
            // reading
            transceive_packet(address_RW >> 1, data, 0, data_size);
        } else {
            // writing
            transceive_packet(address_RW >> 1, data, data_size, 0);
        }
    }
}



/* Read data through I2C. This function sends the following on I2C:
* Start >> (address + write) >> (dataW[0] -> dataW[dataW_size-1]) >> Repeated Start >> (dataR[0] -> dataR[dataR_size-1] >> Stop
* 
* @param address        The address of the device to read from.
* @param data[]         The data to be written to the device (e.g. Register address to read from).
* @param data_size      The size of the data to be written.
* @param read_bytes     The number of bytes to be read. This can be modified later
*                       by returning a value from a receive event.
*/
void transceive_packet(uint8_t address, uint8_t data[], unsigned int data_size, unsigned int read_bytes) {
    Transmission transmission;
    
    
    // transfer data size to pointer
    if (data_size < MAX_DATA_SIZE) {
        transmission.data_size = data_size;
    } else {
        transmission.data_size = MAX_DATA_SIZE;
    }
    transmission.read_bytes = read_bytes;
    
    // check if it is reading or writing
    if (data_size > 0) {
        // writing data, transfer bytes over
        transmission.address_RW = address << 1;
        for (int i = 0; i < data_size && i < MAX_DATA_SIZE; i++) {
            transmission.data[i] = data[i];
        }
    } else {
        // reading data
        transmission.address_RW = (address << 1) | 0b1 ;
    }
    disableInterrupts();
    uint8_t transmissionIndex = 255;
    while (transmissionIndex == 255)
        transmissionIndex = allocate_transmission();
    transmission_pool[transmissionIndex] = transmission;
    
    if (enqueue(transmissionIndex) == 0) {
        free_transmission(transmissionIndex);
    }
    
    
    enableInterrupts();
    
    if (getQueueSize() > MAX_QUEUE_SIZE - 4) {
        // queue is getting full, force an interrupt jic one was missed
        if (isI2CReady()) {
            handleMI2CInterrupt();
        }
    }
    
    
    if (stage == NONE) {
        initiateTransmission();
    }
}

/*  Transmit the next data that needs to be written
 */
void transmitNextData() {
    // make sure read is false
    if (activeTransmission.address_RW | 0b0) {
        I2C1TRN = activeTransmission.data[curDataIndex++];
    }
}

/*  Handles the I2C logic and moves the transmission stage along.
 */
void handleMI2CInterrupt() {
    I2CInterruptQueued = 0;
    // for debugging purposes
    I2C1CON;
    I2C1STAT;
    IFS1;
    
    if (stage == ENABLING && I2C1CONbits.SEN == 0 && I2C1STATbits.S == 1) {
        // Start bit was just sent
        loadNextTransmission();
        // write address
        stage = WRITE_ADDRESS;
        I2C1TRN = activeTransmission.address_RW;
    } else if (stage == DATA && curDataIndex >= activeTransmission.data_size) {
        // In the read section of the data transfer, if there is no data to be read, transfer stops.
        // The index of the data being read is curDataIndex - data_size - 2
        //         write           read
        // size: [data_size][2][read_bytes]
        if (curDataIndex == activeTransmission.data_size && curDataIndex == 0) {
            // data size is 0, jump to reading data
            curDataIndex+= 2;
        }
        if (curDataIndex == activeTransmission.data_size) {
            if (activeTransmission.read_bytes > 0) {
                // just started reading - set RSEN
                curDataIndex++;
                I2C1CONbits.RSEN = 1;
            } else {
                // nothing to read
                stopTransmission();
            }
        } else if (curDataIndex == activeTransmission.data_size + 1) {
            // 2nd flag set after reading - send address
            curDataIndex++;
            I2C1TRN = activeTransmission.address_RW | 0b1;
        } else if (I2C1STATbits.RBF == 1) {
            // receive byte is full - data is ready to be read
            curDataIndex++; 

            // read from the I2CxRCV register
            uint8_t data = I2C1RCV;

            // call events
            uint8_t activeAddress = activeTransmission.address_RW >> 1;
            for (int i = 0; i < numEvents; i++) {
                if (i2cAddresses[i] == activeAddress) {
                    receiveEvent* f = (receiveEvent*) eventAddresses[i];
                    activeTransmission.read_bytes += f(data, activeTransmission.read_bytes + activeTransmission.data_size + 2 - curDataIndex);
                }
            }
            
            
            // generate master acknowledge
            if (activeTransmission.read_bytes + activeTransmission.data_size < curDataIndex - 1) {
                // NACK - last byte in receive has to be this
                I2C1CONbits.ACKDT = 1;
            } else {
                // ACK
                I2C1CONbits.ACKDT = 0;
            }
            I2C1CONbits.ACKEN = 1; // send ACKDT

        } else if (activeTransmission.read_bytes + activeTransmission.data_size < curDataIndex - 1) {
            // no more data to receive
            stopTransmission();
        } else {
            I2C1CONbits.RCEN = 1; // enable receive
        }
    } else if (stage == DISABLING && I2C1CONbits.PEN == 0 && I2C1STATbits.P == 1) {
        if (getQueueSize() > 0) {
            // more data to be sent
            initiateTransmission();
        } else {
            stage = NONE;
        }
    } else if (stage != NONE && _TRSTAT == 0) {
        if (_ACKSTAT == 0) {
            // send next transmission
            if (stage == WRITE_ADDRESS) {
                stage = DATA;
                // check if it is reading
                if (activeTransmission.address_RW & 0b1) {
                    // activate restart bit
                    // Note: There is no repeated start for the BNO085
                    // START ? [Read address + 1] ? Read SHTP Packet ? STOP
                    //I2C1CONbits.RSEN = 1; 
                    
                    if (isI2CReady()) {
                        I2C1CONbits.RCEN = 1; // enable receive
                    } else {
                        // I2C is not ready to continue
                        stage = WRITE_ADDRESS;
                    }
                } else {
                    transmitNextData();
                }
            } else if (stage == DATA) {
                // assert(activeTransmission.data_size > curDataIndex)
                transmitNextData();
            }
        } else {
            // not acknowledged
            stopTransmission();
        }
    }
}

void __attribute__((__interrupt__,__auto_psv__)) _MI2C1Interrupt(void) {
    _MI2C1IF = 0; // clear interrupt
    if (I2CInterruptEnabled) {
        handleMI2CInterrupt();
    } else {
        I2CInterruptQueued = 1;
    }
}

/* Send a write command synchronously through I2C. 
* This will halt all processing until the transmission is complete
* 
* @param address        The address of the device to write to.
* @param data[]         The data to be written to the device.
* @param size           The number of bytes to be written. (size of data)
*/
void write_sync(uint8_t address, uint8_t data[], uint8_t size) {
    while (stage == NONE);

    I2C1CONbits.SEN = 1;
    while (I2C1CONbits.SEN);
    
    I2C1TRN = address << 1;
    while (_TRSTAT && _ACKSTAT);
    
    for (int i = 0; i < size; i++) {
        I2C1TRN = data[i];
        while (_TRSTAT && _ACKSTAT);
    }
    
    I2C1CONbits.PEN = 1;
    while (I2C1CONbits.PEN);
}

/* Initialize the I2C peripheral
 */
void init_i2c() {
    if (initialized)
        return;
    // I2C initialization
    I2C1CON = 0;
    I2C1CONbits.SCLREL = 1; // Clock holding
    I2C1BRG = 0x25;         // 400k Hz baudrate
    _MI2C1IF = 0;           // clear interrupt flag
    _MI2C1IE = 1;           // enable interrupt later
    _MI2C1IP = 6;           // higher interrupt priority
    I2C1CONbits.I2CEN = 1;  // enable
    
    initialized = 1;
}

