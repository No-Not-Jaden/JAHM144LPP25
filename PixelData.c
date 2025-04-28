/* 
 * File:   PixelData.h
 * Author: Mishima
 *
 * Created on April 4, 2025, 9:40 AM
 */

#include "xc.h"
#include "stdint.h"
#include "PixelData.h"
#include <stdlib.h> // for random

#define MAX_BRIGHTNESS 80
#define MIN_BRIGHTNESS 10

typedef struct {
        volatile uint8_t brightness;     // brightness of the LED
        float vx, vy;           // velocity of the LED
        float rx, ry;           // raw relative position
        uint8_t data;
    } LED;

LED leds[ROWS][COLS]; //array for 144 LED pixels
                 //2D array struct [rows][columns] -> [y position][x position]

void setBlink(uint8_t x, uint8_t y, uint8_t blink);
uint8_t isBlink(uint8_t x, uint8_t y);
void setBrightness(uint8_t x, uint8_t y, uint8_t brightness);

void init_pixels(uint8_t numLit){
    
    //clear all LED pixels' properties
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            leds[row][col].rx = 0.0f; //clearing relative x position to its original x position
            leds[row][col].ry = 0.0f; //clearing relative y position to its original y position
            leds[row][col].vx = 0; //clearing x velocity
            leds[row][col].vy = 0; //clearing y velocity
            leds[row][col].brightness = 0; //clearing brightness
            leds[row][col].data = 0;
        }
    }
    
    int litLED = 0;                  //counter for how many LEDs we've lit
    int center = (int) COLS / 2;        //center column of the grid
    
    //initializing starting LED pyramid
    for (int row = 0; row < ROWS && litLED < numLit; row++) {
        int numInRow = 1 + 2 * row; // Row has 1, 3, 5, ... LEDs (centered pyramid)
        int startCol = center - row; // Start column to keep row centered

        for (int i = 0; i < numInRow && litLED < numLit; i++) {
            int col = startCol + i; // Actual column in the grid

            // Only light up if within bounds (avoid out-of-range columns)
            if (col >= 0 && col < COLS) {
                setBrightness(col, row, MIN_BRIGHTNESS + (rand() % (MAX_BRIGHTNESS - MIN_BRIGHTNESS)));
                if (rand() % 3 == 1) {
                    setBlink(col, row, 1);
                }
                litLED++; // Count this LED as lit
            }
        }
    }
    
    T2CON = 0;    
    PR2 = 65535;  
    TMR2 = 0;
    T2CONbits.TCKPS = 0b00;

    T2CONbits.TON = 1;

    IFS0bits.T2IF = 0;
    IPC1bits.T2IP = 2;
    IEC0bits.T2IE = 1;
}

// update brightness for LEDS that blink
void __attribute__((__interrupt__,__auto_psv__)) _T2Interrupt(void)
{
    IFS0bits.T2IF = 0;
    for (uint8_t row = 0; row < ROWS; row++) {
        for (uint8_t col = 0; col < COLS; col++) {
            if (leds[row][col].brightness >= MIN_BRIGHTNESS && isBlink(col, row)) {
                // MSB is direction of fade and rest are the brightness
                uint8_t blinkCount = leds[row][col].brightness & 0x7F;
                if (leds[row][col].brightness >> 7 & 0b1) {
                    // negative direction
                    if (blinkCount == MIN_BRIGHTNESS) {
                        // switch direction
                        leds[row][col].brightness = MIN_BRIGHTNESS;
                    } else {
                        leds[row][col].brightness--;
                    }
                } else {
                    // positive direction
                    if (blinkCount == MAX_BRIGHTNESS) {
                        // switch direction
                        leds[row][col].brightness = 0x80 | MAX_BRIGHTNESS;
                    } else {
                        leds[row][col].brightness++;
                    }
                }
            }
        }
    }
    
    
}

uint8_t getBrightness(uint8_t x, uint8_t y){
        
    return leds[y][x].brightness;
}
    
void setBrightness(uint8_t x, uint8_t y, uint8_t brightness){

    leds[y][x].brightness = brightness;
    
}

float getVelocityX(uint8_t x, uint8_t y){
        
    return leds[y][x].vx;
}
    
float getVelocityY(uint8_t x, uint8_t y){
    
    return leds[y][x].vy;
}
    
void setVelocity(uint8_t x, uint8_t y, float vx, float vy){
    
    leds[y][x].vx = vx;
    leds[y][x].vy = vy;
    
}

float getRawRelativePositionX(uint8_t x, uint8_t y){
    
    return leds[y][x].rx;
}

float getRawRelativePositionY(uint8_t x, uint8_t y){
        
    return leds[y][x].ry;
}

void setRawRelativePosition(uint8_t x, uint8_t y, float rx, float ry){
    
    leds[y][x].rx = rx;
    leds[y][x].ry = ry;
    
}

uint8_t isMoved(uint8_t x, uint8_t y) {
    return leds[y][x].data & 0b1;
}

void setMoved(uint8_t x, uint8_t y, uint8_t moved) {
    if (moved) {
        leds[y][x].data |= 0x01;
    } else {
        leds[y][x].data &= 0xFE;
    }
}

void clearMoved() {
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            setMoved(col, row, 0);
        }
    }
}

uint8_t isBlink(uint8_t x, uint8_t y) {
    return leds[y][x].data >> 1 & 0b1;
}

void setBlink(uint8_t x, uint8_t y, uint8_t blink) {
    if (blink) {
        leds[y][x].data |= 0x02;
    } else {
        leds[y][x].data &= 0xFD;
    }
}

uint8_t getDisplayBrightness(uint8_t x, uint8_t y) {
    if (isBlink(x, y)) {
        return getBrightness(x, y) & 0x7F;
    } else {
        return getBrightness(x, y);
    }
}

uint8_t getData(uint8_t x, uint8_t y) {
    return leds[y][x].data;
}

void setData(uint8_t x, uint8_t y, uint8_t data) {
    leds[y][x].data = data;
}