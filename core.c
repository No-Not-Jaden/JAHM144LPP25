/*
 * File:   core.c
 * Author: boogle
 *
 * Created on April 3, 2025, 9:35 AM
 */


#include "xc.h"


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


void setup() {
    CLKDIVbits.RCDIV = 0;
    
    // Power control - This is so we can reset the devices when the PIC resets
    TRISBbits.TRISB15 = 0; // make RB15 an output
    AD1PCFGbits.PCFG9 = 1; // make RB15 digital
    LATBbits.LATB15 = 0; // disable power to the device
    
    // short delay
    for (int i = 0; i < 1000; i++) {
        asm("nop");
    }
    LATBbits.LATB15 = 1; // enable power to the device
    
    init_i2c();
    bno085_init();
}

int main(void) {
    setup();
    while (1) {
        asm("nop");
    }
    return 0;
}
