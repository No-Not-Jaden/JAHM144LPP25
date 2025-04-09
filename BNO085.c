/*
 * File:   BNO085.c
 * Author: Jaden Paterson
 *
 * Created on March 26, 2025, 2:54 PM
 * 
 * SH-2 Reference: https://www.ceva-ip.com/wp-content/uploads/SH-2-Reference-Manual.pdf
 * SHTP Reference: https://cdn.sparkfun.com/assets/7/6/9/3/c/Sensor-Hub-Transport-Protocol-v1.7.pdf
 */


#include "xc.h"

#include "I2CLib.h"
#include "BNO085.h"
#include "stdio.h"      // For NULL

#define BNO_ADDRESS 0x4A

unsigned int receive_byte(uint8_t, int);
int strncmp(const char* str1, const char* str2);
// initialize I2C on PIC and run initialization sequence on the LCD
void bno085_init() {
    register_event(BNO_ADDRESS, (unsigned long) receive_byte); // register event to be called when there is a byte to be received
    
    // INT0 interrupt
    TRISBbits.TRISB7 = 1; // make RB7 an input pin. RB7 and INT0 pin are multiplexed.
    _INT0EP = 1;  // edge polarity set to detection of negative edge.
    _INT0IF = 0;  // reset so that you don't respond to a previously generated event.
    _INT0IE = 1;  // enable interrupt for INT0 events.
    _INT0IP = 3;  // lower priority
    
    
    if (PORTBbits.RB7 == 0 && _INT0IF == 0) {
        transmit_packet((BNO_ADDRESS << 1) | 0x01, 0, 4); // read SHTP Header
    }
}


void __attribute__((__interrupt__,__auto_psv__)) _INT0Interrupt(void)
{
    // data ready on device
        _INT0IF = 0;
        // dsPIC33/PIC24 FRM, Inter-Integrated Circuit (I2C) Page 24
        // 5.3 Receiving Data from a Slave Device
        // Figure 5-12 page 33 for example
        transmit_packet((BNO_ADDRESS << 1) | 0x01, 0, 4); // read SHTP Header
}




volatile uint8_t buffer[512];
volatile int numBytes = 0;
volatile GravityVector gravityVector;
volatile uint8_t bnoControlChannel = 0xFF;     // Will be set to channel ID of "control"
volatile uint8_t bnoInputChannel = 0xFF;       // Will be set to channel ID of "inputNormal"
volatile uint8_t bnoDeviceChannel = 0xFF;      // Will be set to channel ID of "device"
volatile uint8_t send_sequence = 0;
volatile int resetStatus = 0;
#define Q14_SCALE (1.0f / 16384.0f) // Q14 means the number should be divided by 2^14
#define MIN_ACCURACY 2
#define GRAVITY_REPORT_INTERVAL 0xC350 // in microseconds (50000 us = 20Hz)

void enable_gravity_vector(){
    // https://www.ceva-ip.com/wp-content/uploads/SH-2-Reference-Manual.pdf
    // 6.5.4 Set Feature Command (Page 65)
    uint8_t data[] = {
                    0x15, 0x00,        // SHTP header (length = 21 bytes) LSB then MSB
                    bnoControlChannel,              // Channel (Sensor Hub Control)
                    send_sequence++,              // Sequence number
                    0xFD,              // Set Feature Command
                    0x06,              // Gravity Vector Feature Report ID
                    0x00,        // Feature flags (0 = default)
                    0x00, 0x00,        // Change sensitivity (disabled)
                    ((uint8_t) (GRAVITY_REPORT_INTERVAL & 0xFF)), // 32-bit Report interval in microseconds LSB to MSB
                    ((uint8_t) (GRAVITY_REPORT_INTERVAL >> 8 & 0xFF)),
                    ((uint8_t) (GRAVITY_REPORT_INTERVAL >> 16 & 0xFF)),
                    ((uint8_t) (GRAVITY_REPORT_INTERVAL >> 24 & 0xFF)),  
                    0x00, 0x00, 0x00, 0x00,  // Batch interval (disabled)
                    0x00, 0x00, 0x00, 0x00   // Sensor-specific config (default)
                  };
    transmit_packet(BNO_ADDRESS << 1, data, 21);
}

int strncmp(const char* str1, const char* str2) {
    char* tmpPointer1 = str1;
    char* tmpPointer2 = str2;
    while (*tmpPointer1 == *tmpPointer2 && *tmpPointer1 != 0x00 && *tmpPointer2 != 0x00) {
        tmpPointer1++;
        tmpPointer2++;
    }
    return *tmpPointer1 == *tmpPointer2;
}

// appName must end with null char
int findSH2App(const char* appName) {
    int index = 4; // skip SHTP header
    // look for 0x08, (byte), appName
    while (index < numBytes) {
        while (buffer[index] != 0x08 && index < numBytes) {
            // increment index until we reach 0x08 = App Name
            index++;
        }
        
        if (index + 11 > numBytes) {
            // not enough bytes remaining
            return -1;
        }
        
        if (strncmp(&buffer[index + 2], appName)) {
            // found app
            break;
        }
        index++;
    }
    if (index >= numBytes) {
        // did not find app
        return -1;
    }
    return index;
}

/* Get the next SH-2 channel in the buffer
 * @param *channelNum   Address of a variable to set the channel number to.  
 * @param *index        Address of the index of where to start in the buffer.
 *                      This will be updated to the end of the next channel.
 * Returns channel name, or null if none left in the buffer.
 */
char* readNextChannel(uint8_t* channelNum, int* index){
    char* channelName = NULL;
    if (buffer[*index] == 0x08)
        (*index)++;
    // looking for 2 elements
    for (int i = 0; i < 2; i++) {
        // look for Normal Channel Tag or Channel Name Tag
        while (buffer[*index] != 0x06 && buffer[*index] != 0x09 && *index < numBytes) {
            (*index)++;
        }
        if (*index >= numBytes) {
            // no more channels
            return NULL;
        }
        if (buffer[*index] == 0x06) {
            *channelNum = buffer[*index+2]; // this sets the value of channelNum to the element in the buffer
        } else {
            channelName = &buffer[*index+2]; // this sets the pointer of channelName to the location in the buffer
        }
        (*index)++;
    }
    return channelName;
}

/* Reads the SH-2 SHTP advertisement and saves desired channels to global variables
 * https://www.ceva-ip.com/wp-content/uploads/SH-2-SHTP-Reference-Manual.pdf
 */
int readAdvertisement() {
    // find sensorhub app for control and inputNormal channels
    int index = findSH2App("sensorhub\0");
    if (index == -1) {
        // app not found
        return -1;
    }
    uint8_t nextChannelNumber = 0;
    char* nextChannelName = readNextChannel(&nextChannelNumber, &index);
    while (nextChannelName != NULL) {
        if (strncmp(nextChannelName, "control\0")) {
            bnoControlChannel = nextChannelNumber;
        } else if (strncmp(nextChannelName, "inputNormal\0")) {
            bnoInputChannel = nextChannelNumber;
        }
        nextChannelName = readNextChannel(&nextChannelNumber, &index);
    }
    
    // find executable app for device channel
    index = findSH2App("executable\0");
    if (index == -1) {
        // app not found
        return -1;
    }
    nextChannelName = readNextChannel(&nextChannelNumber, &index);
    while (nextChannelName != NULL) {
        if (strncmp(nextChannelName, "device\0")) {
            bnoDeviceChannel = nextChannelNumber;
        }
        nextChannelName = readNextChannel(&nextChannelNumber, &index);
    }
    
    return bnoControlChannel != 0xff && bnoInputChannel != 0xff && bnoDeviceChannel != 0xff;
    
}

void processMessage() {
    uint8_t channel = buffer[2];
    uint8_t sequence = buffer[3];
    if (channel == 0x00 && sequence == 0x00) {
        // command channel and first message
        int result = readAdvertisement();
        if (result == 1) {
            resetStatus++;
            
        }
    } else if (channel == bnoDeviceChannel && sequence == 0x00) {
        // check for restart confirmation
        if (buffer[4] == 1) {
            resetStatus++;
        }
    } else if (channel == bnoInputChannel) {
        // sensor report (Gravity)
        // https://www.ceva-ip.com/wp-content/uploads/SH-2-Reference-Manual.pdf
        // 6.5.11 Gravity (Page 68)
        // 7.2.1 Base Timestamp Reference (Page 93)
        uint8_t reportID = buffer[4]; // 0xFB for base timestamp
        int index = 5;
        long baseDelta = 0;
        if (reportID == 0xFB) {
           // relative to transport-defined reference point. Signed. Units are 100 microsecond ticks
           baseDelta = (long) buffer[index++];
           baseDelta |= (long) buffer[index++] << 8;
           baseDelta |= (long) buffer[index++] << 16;
           baseDelta |= (long) buffer[index++] << 24;
           reportID = buffer[index++];
        }
        if (reportID == 0x06) {
            uint8_t sequenceNum = buffer[index++]; // specific for this report
            uint8_t status = buffer[index++] & 0x03; // 0-3 for accuracy
            // buffer[7] is delay
            if (status < MIN_ACCURACY)
                return;

            unsigned int rawX = (unsigned int) buffer[index++];
            rawX |= (unsigned int) buffer[index++] << 8;
            unsigned int rawY = (unsigned int) buffer[index++];
            rawY |= (unsigned int) buffer[index++] << 8;
            unsigned int rawZ = (unsigned int) buffer[index++];
            rawZ |= (unsigned int) buffer[index++] << 8;

            // converts to floating point
            if (gravityVector.average_count == 0) {
                // have not stored anything here yet
                gravityVector.x = ((int) rawX) * Q14_SCALE;
                gravityVector.y = ((int) rawY) * Q14_SCALE;
                gravityVector.z = ((int) rawZ) * Q14_SCALE;
            } else {
                // merge with previous values
                float prevMultiplier = gravityVector.average_count / (gravityVector.average_count + 1.0f);
                float curMultiplier = 1 / (gravityVector.average_count + 1.0f);
                gravityVector.x = prevMultiplier * gravityVector.x + curMultiplier * (((int) rawX) * Q14_SCALE);
                gravityVector.y = prevMultiplier * gravityVector.y + curMultiplier * (((int) rawY) * Q14_SCALE);
                gravityVector.z = prevMultiplier * gravityVector.z + curMultiplier * (((int) rawZ) * Q14_SCALE);
            }
            gravityVector.deltaTime += baseDelta;
            gravityVector.average_count++;
        }
    }
    
    if (resetStatus == 2) {
        resetStatus++;
        enable_gravity_vector();
    }
    
    numBytes = 0;
}

void getGravityVector(GravityVector* out) {
    // copy over gravity vector values
    out->x = gravityVector.x;
    out->y = gravityVector.y;
    out->z = gravityVector.z;
    out->deltaTime = gravityVector.deltaTime;
    out->average_count = gravityVector.average_count;
    // reset vector
    gravityVector.deltaTime = 0;
    gravityVector.average_count = 0;
}


unsigned int receive_byte(uint8_t byte, int remainingBytes) {
    buffer[numBytes++] = byte;
    if (numBytes == 2) {
        // read total packet length
        unsigned int byteIncrease = buffer[0] + (((unsigned int) buffer[1]) << 8);
        return byteIncrease - 4; // 4 bytes is the size of the SHTP header
    }
    
    if (remainingBytes == 0) {
        processMessage();
    }
    
    return 0;
}
