/*
 * File:   core.c
 * Author: boogle
 *
 * Created on April 3, 2025, 9:35 AM
 */


#include "xc.h"
#include "BNO085.h"
#include "lcd.h"
#include "I2CLib.h"
#include <math.h>


// CW1: FLASH CONFIGURATION WORD 1 (see PIC24 Family Reference Manual 24.1)
#pragma config ICS = PGx1          // Comm Channel Select (Emulator EMUC1/EMUD1 pins are shared with PGC1/PGD1)
#pragma config FWDTEN = OFF        // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config GWRP = OFF          // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF           // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF        // JTAG Port Enable (JTAG port is disabled)


// CW2: FLASH CONFIGURATION WORD 2 (see PIC24 Family Reference Manual 24.1)
#pragma config I2C1SEL = PRI       // I2C1 Pin Location Select (Use default SCL1/SDA1 pins)
#pragma config IOL1WAY = OFF       // IOLOCK Protection (IOLOCK may be changed via unlocking seq)
#pragma config OSCIOFNC = ON       // Primary Oscillator I/O Function (CLKO/RC15 functions as I/O pin)
#pragma config FCKSM = CSECME      // Clock Switching and Monitor (Clock switching is enabled, 
                                       // Fail-Safe Clock Monitor is enabled)
#pragma config FNOSC = FRCPLL      // Oscillator Select (Fast RC Oscillator with PLL module (FRCPLL))

GravityVector vector;
void normalize(GravityVector* vector);

void setup() {
    CLKDIVbits.RCDIV = 0;
    
    // Power control - This is so we can power cycle the devices when the PIC resets
    TRISBbits.TRISB15 = 0; // make RB15 an output
    AD1PCFGbits.PCFG9 = 1; // make RB15 digital
    LATBbits.LATB15 = 0; // disable power to the device
    
    // short delay
    delay(50);
    LATBbits.LATB15 = 1; // enable power to the device
    
    init_i2c();
    bno085_init();
    delay(1000);
    lcd_init();
    
}

int main(void) {
    setup();
    
    while (1) {
        // get the gravity vector
        getGravityVector(&vector);
        // normalize it
        normalize(&vector);
        
        
    }
    return 0;
}

// Normalizes the gravity vector
void normalize(GravityVector* vector) {
    float magnitude = sqrt(vector->x * vector->x + vector->y * vector->y + vector->z * vector->z);
    vector->x = vector->x / magnitude;
    vector->y = vector->y / magnitude;
    vector->z = vector->z / magnitude;
}

// returns the X acceleration of the normalized gravity vector
float getAccelerationX() {
    return vector.x * 9.8f;
}

// returns the Y acceleration of the normalized gravity vector
float getAccelerationY() {
    return vector.y * 9.8f;
}

void displayGravityVector() {
    lcd_clear();
    lcd_set_cursor(0,0);
    if (vector.average_count == 0) {
        lcd_write_string("No Data\0");
    } else {
        char str[20];
        sprintf(str, "%2.1f %2.1f", vector.x, vector.y);
        lcd_write_string(&str);
        lcd_set_cursor(1,0);
        char str2[20];
        sprintf(str2, "%2.1f %d", vector.z, getTransmissionsUsed());
        lcd_write_string(&str2);
        delay(100);
    }
}
