/*
 * File:   lcd.c
 * Author: jpate
 *
 * Created on April 16, 2025, 2:15 PM
 */


#include "xc.h"

#define DOGS104_ADDR    0x3C     // 7-bit I²C address
#define LCD_CMD         0x00     // Control byte for commands
#define LCD_DATA        0x40     // Control byte for data

void lcd_send_packet(const uint8_t* cmds, uint8_t length, uint8_t controlByte) {
    uint8_t packet[length * 2];
    for (uint8_t i = 0; i < length; i++) {
        if (i < length - 1) {
            packet[i * 2] = controlByte | 0b10000000; // Add continuation bit
        } else {
            packet[i * 2] = controlByte;
        }
        packet[i * 2 + 1] = cmds[i];
    }
    transmit_packet(DOGS104_ADDR << 1, packet, length * 2);
}

void delay(int delay_in_ms) {
    for (int i = 0; i < delay_in_ms; i++) {
        for (int j = 0; j < 1770; j++) {
            asm("nop");
        }
    }
}

void lcd_clear() {
    uint8_t cmds[] = { 0x01 }; // Clear display
    lcd_send_packet(cmds, sizeof(cmds), LCD_CMD);
}

void lcd_set_cursor(uint8_t row, uint8_t col) {
    uint8_t cmd = 0b10000000 | (0x20 * row + col);
    lcd_send_packet(&cmd, 1, LCD_CMD);
}

void lcd_write_char(char c) {
    lcd_send_packet(c, 1, LCD_DATA);
}

void lcd_write_string(const char* str) {
    uint8_t buffer[32];
    uint8_t len = 0;

    while (*str && len < 32) {
        buffer[len] = (uint8_t)(*str++);
        len++;
    }

    lcd_send_packet(buffer, len, LCD_DATA);
}

void lcd_init() {
    TRISBbits.TRISB6 = 0; // Reset pin as output

    // Hardware reset
    LATBbits.LATB6 = 1;
    delay(50);
    LATBbits.LATB6 = 0;
    delay(1);
    LATBbits.LATB6 = 1;
    delay(100);

    // Initialization sequence
    const uint8_t init_sequence[] = {
        0x3A, // 8-bit, extension set (RE=1)
        0x09, // 4-line display
        0x06, // Bottom view
        0x1E, // Bias set BS1=1
        0x39, // Extension set RE=0, IS=1
        0x1B, // Bias 1/6
        0x6E, // Divider ON
        0x56, // Booster on and contrast (C5,C4)
        0x7A, // Contrast (C3-C0)
        0x38, // Return to normal RE=0 IS=0
        0x0F,  // Display ON, cursor ON, blink ON
        0x3A, // Extended Function Set
        0x09, // 4-line display
        0x1A, // Double height/bias adjustment
        0x3C  // Return to standard mode
    };
    lcd_send_packet(init_sequence, sizeof(init_sequence), LCD_CMD);

    delay(5); // Small delay

    lcd_clear(); // Clear screen
}