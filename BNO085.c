/*
 * File:   BNO085.c
 * Author: Jaden Paterson
 *
 * Created on March 26, 2025, 2:54 PM
 */


#include "xc.h"

#include "I2CLib.h"
#include "BNO085.h"
#include <string.h>

#define BNO_ADDRESS 0x4A

unsigned int receive_byte(uint8_t, int);

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
volatile Vector3d gravityVector;
volatile uint8_t bnoControlChannel = 0xFF;     // Will be set to channel ID of "control"
volatile uint8_t bnoInputChannel = 0xFF;       // Will be set to channel ID of "inputNormal"
volatile uint8_t send_sequence = 0;
#define Q14_SCALE (1.0f / 16384.0f)
#define MIN_ACCURACY 2

void enable_gravity_vector(){
    uint8_t data[] = {
                    0x16, 0x00,        // SHTP header (length = 22 bytes) LSB then MSB
                    bnoControlChannel,              // Channel (Sensor Hub Control)
                    send_sequence++,              // Sequence number
                    0xFD,              // Set Feature Command
                    0x06,              // Gravity Vector Feature Report ID
                    0x00, 0x00,        // Feature flags (0 = default)
                    0x00, 0x00,        // Change sensitivity (disabled)
                    0xE8, 0x03, 0x00, 0x00,  // Report interval (0x03E8 = 1000 µs = 1kHz)
                    0x00, 0x00, 0x00, 0x00,  // Batch interval (disabled)
                    0x00, 0x00, 0x00, 0x00   // Sensor-specific config (default)
                  };
    transmit_packet(BNO_ADDRESS << 1, data, 22);
}

int parseBNOChannelMap() {
    if (numBytes < 4) return -1;

    size_t index = 4; // Skip SHTP header

    while (index + 2 < numBytes) {
        if (buffer[index] != 0x08) break; // Expect "executable" app type

        uint8_t channelId = buffer[index + 1];
        uint8_t nameStart = index + 2;

        // Get length of null-terminated string
        size_t nameLen = 0;
        while ((nameStart + nameLen < numBytes) && buffer[nameStart + nameLen] != 0x00) {
            nameLen++;
            if (nameLen >= 32) break; // safety cap
        }

        if (nameStart + nameLen >= numBytes) break; // avoid overflow

        // Compare names
        if (strncmp((const char*)&buffer[nameStart], "control", nameLen) == 0) {
            bnoControlChannel = channelId;
        } else if (strncmp((const char*)&buffer[nameStart], "inputNormal", nameLen) == 0) {
            bnoInputChannel = channelId;
        }

        index = nameStart + nameLen + 1; // Move to next entry
    }

    return 0; // success
}

void processMessage() {
    uint8_t channel = buffer[2];
    uint8_t sequence = buffer[3];
    if (channel == 0x00 && sequence == 0x00) {
        // command channel and first message
        int result = parseBNOChannelMap();
        if (result == 0) {
            enable_gravity_vector();
        }
    } else if (channel == bnoInputChannel) {
        // sensor report
        uint8_t reportID = buffer[4]; // 0x06 for gravity vector
        uint8_t sequenceNum = buffer[5]; // specific for this report
        uint8_t status = buffer[6] & 0x03; // 0-3 for accuracy
        if (status < MIN_ACCURACY)
            return;
        
        unsigned int rawX = (unsigned int)(buffer[8] | (buffer[9] << 8));
        unsigned int rawY = (unsigned int)(buffer[10] | (buffer[11] << 8));
        unsigned int rawZ = (unsigned int)(buffer[12] | (buffer[13] << 8));
        
        // converts to floating point
        gravityVector.x = rawX * Q14_SCALE;
        gravityVector.y = rawY * Q14_SCALE;
        gravityVector.z = rawZ * Q14_SCALE;
        
    }
    
    numBytes = 0;
}

void getGravityVector(Vector3d* out) {
    out->x = gravityVector.x;
    out->y = gravityVector.y;
    out->z = gravityVector.z;
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
