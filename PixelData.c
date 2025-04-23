
#include "xc.h"
#include "stdint.h"
#include "PixelData.h"

LED leds[ROWS][COLS];

void init_pixels(uint8_t numLit){
    
    //clear all LED pixels' properties
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            leds[row][col].rx = 0.0f; //clearing relative x position to its original x position
            leds[row][col].ry = 0.0f; //clearing relative y position to its original y position
            leds[row][col].vx = 0; //clearing x velocity
            leds[row][col].vy = 0; //clearing y velocity
            leds[row][col].brightness = 0; //clearing brightness
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
                leds[row][col].brightness = 40; // Light up this LED
                litLED++; // Count this LED as lit
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