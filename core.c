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
#include "PixelData.h"
#include "PositionCalculator.h"
#include "LED_144_Lib.h"


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

#define ACCEL_MULTIPLIER 1.5

GravityVector vector;
void normalize(GravityVector* vector);

// delay roughly an amount of time in milliseconds
void delay(int delay_in_ms) {
    for (int i = 0; i < delay_in_ms; i++) {
        for (int j = 0; j < 1770; j++) {
            asm("nop");
        }
    }
}

void setup() {
    CLKDIVbits.RCDIV = 0;
    
    // Power control - This is so we can power cycle the devices when the PIC resets
    TRISBbits.TRISB15 = 0; // make RB15 an output
    AD1PCFGbits.PCFG9 = 1; // make RB15 digital
    LATBbits.LATB15 = 0; // disable power to the device
    
    // short delay
    delay(50);
    LATBbits.LATB15 = 1; // enable power to the device
    
    TRISBbits.TRISB6 = 0;
    
    init_i2c();
    bno085_init();
    delay(500);
    init_pixels(49);
    //lcd_init();
    led_init();
    write_all();
    delay(500);
    
    LATBbits.LATB6 = 1; // signal that setup is complete
}

int main(void) {
    setup();
    
    while (1) {
        // get the acceleration vector
        getAccVector(&vector);
        
        if (vector.average_count > 0) {
            
            // apply acceleration
            float ax = vector.x * ACCEL_MULTIPLIER;
            float ay = vector.y * ACCEL_MULTIPLIER;
            for (uint8_t row = 0; row < ROWS; row++) {
                for (uint8_t col = 0; col < COLS; col++) {
                    applyAcceleration(col, row, ay, ax, vector.deltaTime);
                }
            }
            // display LEDS on device
            write_all();
            clearMoved();
        }
        // delay for next update
        delay(10);
        //displayGravityVector();
        delay(10);
        
    }
    return 0;
}

// displays the vector variable on the LCD
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
        sprintf(str2, "%2.1f %d", vector.z, vector.average_count);
        lcd_write_string(&str2);
        
    }
}
