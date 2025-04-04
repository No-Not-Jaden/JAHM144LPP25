/* 
 * File:   PositionCalculator.h
 * Author: your mom
 *
 * Created on April 4, 2025, 9:18 AM
 */

#ifndef POSITIONCALCULATOR_H
#define	POSITIONCALCULATOR_H

#ifdef	__cplusplus
extern "C" {
#endif
    
    /* Check if a position is open relative to another. AKA checking if the
     * pixel at (x + dx, y + dy) is lit already.
     * 
     * @param x     The x position of the origin point.
     * @param y     The y position of the origin point.
     * @param dx    The change in x position. (Should be -1, 0, or 1)
     * @param dy    The change in y position. (Should be -1, 0, or 1)
     */
    uint8_t isOpen(uint8_t x, uint8_t y, int dx, int dy);
    
    /* Move a pixel to another position. The pixel at the (xTo, yTo) position
     * will be overwritten with the data from the pixel at the (xFrom, yFrom) 
     * position. Then, the data is erased  at (xFrom, yFtom).
     * 
     * @param xFrom     The x position of the pixel to be moved.
     * @param yFrom     The y position of the pixel to be moved.
     * @param xTo       The new x position of the pixel.
     * @param yTo       The new y position of the pixel.
     */
    void movePixel(uint8_t xFrom, uint8_t yFrom, uint8_t xTo, uint8_t yTo);
    
    /* Try to move a pixel in a direction. If there is already a pixel in the
     * direction to move to, the pixels on either side of that pixel will be 
     * checked. If none of those positions are empty, the pixel will not be moved.
     * 
     * @param x     The x position of the pixel to be moved.
     * @param y     The y position of the pixel to be moved.
     * @param dx    The change in x position. (Should be -1, 0, or 1)
     * @param dy    The change in y position. (Should be -1, 0, or 1)   
     */
    void tryMovePixel(uint8_t x, uint8_t y, int dx, int dy);
    
    /* Apply acceleration to a pixel and possibly move it. The acceleration will
     * be converted into velocity(acceleration * change in time) that is added 
     * to the current velocity of the pixel, and the current velocity will be
     * converted to a change in position (velocity * change in time) which will
     * be added to the current pixel's raw position. If the x or y value of the
     * raw relative position is greater than 1, then the pixel will attempt to
     * move.
     * 
     * dx = vx*dt + (1/2)*ax*t^2
     * 
     * @param x     The x position of the pixel to apply the acceleration to.
     * @param y     The y position of the pixel to apply acceleration to.
     * @param ax    The x component of the acceleration being applied to the pixel.
     * @param xy    The y component of the acceleration being applied to the pixel.
     * @param dt    The change in time since the last acceleration (in milliseconds).
     */
    void applyAcceleration(uint8_t x, uint8_t y, float ax, float ay, unsigned int dt);


#ifdef	__cplusplus
}
#endif

#endif	/* POSITIONCALCULATOR_H */

