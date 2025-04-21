/*
 * File:   lcd.c
 * Author: jpate
 *
 * Created on April 16, 2025, 2:15 PM
 */


#include "xc.h"
#define DOGS104_ADDR    0x3C     // 7-bit I²C address
#define LCD_CMD         0x00     // Control byte: command
#define LCD_DATA        0x40     // Control byte: data

void lcd_cmd(uint8_t cmd) {
    uint8_t packet[] = { LCD_CMD, cmd };
    transmit_packet(DOGS104_ADDR << 1, packet, 2);
}

void lcd_clear() {
    lcd_cmd(1);
}

void lcd_send_data(uint8_t data) {
    uint8_t packet[] = { LCD_DATA, data };
    transmit_packet(DOGS104_ADDR << 1, packet, 2);
}

// delays the processor for a certain number of milliseconds
void delay(int delay_in_ms) {
    for (int i = 0; i < delay_in_ms; i++) {
        for (int j = 0; j < 1770; j++) {
                asm("nop");
        }
    }
}

// initialize I2C on PIC and run initialization sequence on the LCD
void lcd_init() {
    TRISBbits.TRISB6 = 0; // set reset pin as output

    // reset LCD
    // ~50 ms high
    LATBbits.LATB6 = 1;
    delay(50); 
    // 0.2 ms low
    LATBbits.LATB6 = 0;
    delay(1);
    // high
    LATBbits.LATB6 = 1;
    delay(100);
    
    // initialize LCD
    lcd_cmd(0x3a); // 8 bit data length extension Bit RE=1; REV=0
    lcd_cmd(0x09); // 4 line display
    lcd_cmd(0x06); // bottom view
    lcd_cmd(0x1e); // BS1=1
    lcd_cmd(0x39); // 8 bit data length extension Bit RE=0; IS=1
    lcd_cmd(0x1b); // BS0=1 -> Bias=1/6
    lcd_cmd(0x6e); // Devider on and set value
    lcd_cmd(0x56); // Booster on and set contrast (DB1=C5, DB0=C4)
    lcd_cmd(0x7A); // Set contrast (DB3-DB0=C3-C0)
    lcd_cmd(0x38); // 8 bit data length extension Bit RE=0; IS=0
    lcd_cmd(0x0F); // Display on, cursor on, blink on
    
    /* Function set (RE=1 version) */
    lcd_cmd(0x3a); /* DL, N, ~BE, enter extended mode RE=1, ~REV */ 
    /* Extended function set (assumes RE=1) */
    lcd_cmd(0x09); /* NW, ~FW, ~B/W */
    /* Double-height/bias/dot-shift (assumes RE=1) */
    lcd_cmd(0x1a); /* UD2, ~UD1, BS1, ~DH? */
    /* Function set (RE=0 version, IS=0) */
    lcd_cmd(0x3c); /* DL, N, DH, return to RE=0, ~IS */

    lcd_cmd(1); // clear display
    
}


void lcd_set_cursor(uint8_t row, uint8_t col) {
    lcd_cmd(0b10000000 | (0x20 * row + col));
}

void lcd_write_char(char c) {
    lcd_send_data((uint8_t)c);
}

void lcd_write_string(const char* str) {
    while (*str) {
        lcd_write_char(*str++);
    }
}