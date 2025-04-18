/*
 * Contains functions for interfacing with the IS31FL3731 display
 * Alexander Younk
 * April 17th, 2025
 * This file allows the code to communicate with the IS31FL3731 display
 * It utilizes I2C to send various commands to the display
 * For more information on how the I2C communication works, see I2CLib.h/.c
 * MAKE SURE TO ADD "I2CLib.h" AND "queue.h" to compile the code
 */

#include "xc.h"
#include <stdio.h>
#include "LED_144_Lib.h"
#include "stdint.h"

//Define the slave address, 0xE8
#define SLAVE_ADDRESS 0b11101000

/*
 * This function doesn't take in arguments or return anything
 * This function sends a variety of I2C commands to initialize the display
 * The function register is chosen by sending 0xFD, and then choosing a frame
 * Afterwards, the I2C commands will write to that frame until it's changed
 * For specifics on individual commands, see the code below
 */
void led_init(void){
    
    uint8_t data[] = {
        0xFD, //pick a frame 
        0x0B //Go to the function register
    };
    
    transmit_packet(SLAVE_ADDRESS, data, 2); 
    
    data[0] = 0x00; //configuration register
    data[1] = 0x00; //picture mode
    transmit_packet(SLAVE_ADDRESS, data, 2);
 
    data[0] = 0x01; //picture display register
    data[1] = 0x00; //go to frame one
    transmit_packet(SLAVE_ADDRESS, data, 2);
     
    data[0] = 0x0A; //go to the shut down register
    data[1] = 0x00; //shutdown
    transmit_packet(SLAVE_ADDRESS, data, 2);
    
    data[0] = 0x0A; //Go to shutdown register
    data[1] = 0x01; //normal operation
    transmit_packet(SLAVE_ADDRESS, data, 2);
    
    data[0] = 0xFD; //Choose a frame
    data[1] = 0x00; //Go to frame 1
    transmit_packet(SLAVE_ADDRESS, data, 2);
    
   int i = 0; //turn off all the led's
    for (i = 0; i<0x12; i++){
        data[0] = i;
        data[1] = 0x00;
        transmit_packet(SLAVE_ADDRESS, data, 2);
    }
}

/*
 * This function writes to a specific led with a certain brightness
 * It takes in three integers for the position and the brightness
 * IMPORTANT: Range for inputs
 * ROW: 1-9
 * COLUMN: 1-16
 * BRIGHTNESS: 0-255
 * DO NOT GO OUTSIDE THESE RANGES
 * THE BRIGHTNESS INCREASES VERY QUICKLY, PROBABLY DON'T NEED MORE THAN 100
 * FEEL FREE TO EXPERIMENT WITH THE BRIGHTNESS, YOU'VE BEEN WARNED
 * This function begins by selecting frame 1
 * Then it chooses which led with various mathematical commands
 * The position (row and column) are determined the same way
 * After all the data is configured, send it through I2C
 */
void led_write(uint8_t row, uint8_t column, uint8_t brightness){
    uint8_t data[] = {
      0xFD, //Select a frame register
      0x00 //Go to frame one
    };
    transmit_packet(SLAVE_ADDRESS, data,2);
    
    //Change the brightness
    if(column < 9) {
        data[0] = (((row*16)+20)+(column-1));
    }
    else{
        data[0] = (((row*16)+28)+(column-9));
    }
    
    data[1] = brightness;
    transmit_packet(SLAVE_ADDRESS, data,2);
    
    //Determine the matrix and row we're writing to
    if(column < 9){
        data[0] = ((row*2)-2);
    } 
    else {
        data[0] = ((row*2)-1);
    }
    
    //Determine the led (column) that we're writing to
    if(column < 9){
        data[1] = 0x01 << (column-1);
    }
    else{
        data[1] = 0x01 << (column-9);
    }
    transmit_packet(SLAVE_ADDRESS, data, 2);
   
}