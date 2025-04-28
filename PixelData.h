/* 
 * File:   PixelData.h
 * Author: Mishima
 *
 * Created on April 4, 2025, 9:40 AM
 */

#ifndef PIXELDATA_H
#define	PIXELDATA_H

#ifdef	__cplusplus
extern "C" {
#endif
    
    #define ROWS 9
    #define COLS 16

    /* Initialize the pixel data with some pixels lit.
     * 
     * @param numLit    The number of pixels to be lit initially.
     */
    void init_pixels(uint8_t numLit);
    
    /* Get the brightness of a pixel at a position
     * 
     * @param x     The x position of the pixel.
     * @param y     The y position of the pixel.
     */
    uint8_t getBrightness(uint8_t x, uint8_t y);
    
    /* Set the brightness of a pixel.
     * 
     * @param x             The x position of the pixel.
     * @param y             The y position of the pixel.
     * @param brightness    The brightness of the pixel.
     */
    void setBrightness(uint8_t x, uint8_t y, uint8_t brightness);
    
    /* Get the X component of the velocity of a pixel.
     * 
     * @param x     The x position of the pixel.
     * @param y     The y position of the pixel.
     */
    float getVelocityX(uint8_t x, uint8_t y);
    
    /* Get the Y component of the velocity of a pixel.
     * 
     * @param x     The x position of the pixel.
     * @param y     The y position of the pixel.
     */
    float getVelocityY(uint8_t x, uint8_t y);
    
    /* Set the velocity of the pixel.
     * 
     * @param x     The x position of the pixel.
     * @param y     The y position of the pixel.
     * @param vx    The x component of the velocity.
     * @param vy    The y component of the velocity.
     */
    void setVelocity(uint8_t x, uint8_t y, float vx, float vy);
    
    /* Get the X component of the raw relative position of a pixel.
     * 
     * @param x     The x position of the pixel.
     * @param y     The y position of the pixel.
     */
    float getRawRelativePositionX(uint8_t x, uint8_t y);
    
    /* Get the Y component of the raw relative position of a pixel.
     * 
     * @param x     The x position of the pixel.
     * @param y     The y position of the pixel.
     */
    float getRawRelativePositionY(uint8_t x, uint8_t y);
    
    /* Set the raw relative position of a pixel
     * 
     * @param x     The x position of the pixel.
     * @param y     The y position of the pixel.
     * @param rx    The x component of the relative position of the pixel.
     * @param ry    The y component of the relative position of the pixel
     */
    void setRawRelativePosition(uint8_t x, uint8_t y, float rx, float ry);
    
    /* Check if a certain pixel has the moved attribute.
     * 
     * @param x     The x position of the pixel.
     * @param y     The y position of the pixel.
     * @returns     True if the pixel has the moved attribute.
     */
    uint8_t isMoved(uint8_t x, uint8_t y);
    
    /* Set a pixel's moved attribute.
     * 
     * @param x     The x position of the pixel.
     * @param y     The y position of the pixel.
     * @param moved The value to set the moved attribute to.
     */
    void setMoved(uint8_t x, uint8_t y, uint8_t moved);
    
    /* Clear the moved attribute on all pixels.
     */
    void clearMoved();
    
    /* Get the data for a pixel. This holds the pixel attributes.
     * 
     * @param x     The x position of the pixel.
     * @param y     The y position of the pixel.
     * @returns     The data of the pixel.
     */
    uint8_t getData(uint8_t x, uint8_t y);
    
    /* Set the data of the pixel.
     * 
     * @param x     The x position of the pixel.
     * @param y     The y position of the pixel.
     * @param data  The new data for the pixel.
     */
    void setData(uint8_t x, uint8_t y, uint8_t data);
    
    /* Get the brightness that should be displayed for this pixel.
     * The regular brightness may be modified according to the pixel attributes.
     * 
     * @param x     The x position of the pixel.
     * @param y     The y position of the pixel.
     * @returns     The brightness of the pixel ready to be displayed.
     */
    uint8_t getDisplayBrightness(uint8_t x, uint8_t y);


#ifdef	__cplusplus
}
#endif

#endif	/* PIXELDATA_H */

