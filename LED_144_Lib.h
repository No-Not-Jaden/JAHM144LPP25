/* 
 * File:   LED_144_Lib.h
 * Author: alexb
 *
 * Created on April 17, 2025, 8:51 AM
 */

#ifndef LED_144_LIB_H
#define	LED_144_LIB_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "PixelData.h"

    /*
    * This function doesn't take in arguments or return anything
    * This function sends a variety of I2C commands to initialize the display
    * The function register is chosen by sending 0xFD, and then choosing a frame
    * Afterwards, the I2C commands will write to that frame until it's changed
    * For specifics on individual commands, see the code below
    */
    void led_init(void);
    
    /*
    * This function writes to a specific led with a certain brightness
    * It takes in three integers for the position and the brightness
    * IMPORTANT: Range for inputs
    * ROW: 1-9
    * COLUMN: 1-16
    * BRIGHTNESS: 0-255
    * DO NOT GO OUTSIDE THESE RANGES
    * This function begins by selecting frame 1
    * Then it chooses which led with various mathematical commands
    * The position (row and column) are determined the same way
    * After all the data is configured, send it through I2C
    */
    void led_write(uint8_t row, uint8_t column, uint8_t brightness);
    
    /*
    * The LED write_all function begins by initializing the data array to zero
    * Next, frame 1 is chosen
    * Afterwards, the all the LED's in every row are turned on.
    * Finally, the brightness of every led is changed according to the gravity vector
    */
    void write_all ();

#ifdef	__cplusplus
}
#endif

#endif	/* LED_144_LIB_H */

