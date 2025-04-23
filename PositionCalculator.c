/*
 * File:   PositionCalculator.c
 * Author: Christian
 *
 * Created on April 21, 2025, 11:01 AM
 */


#include "xc.h"
#include "PixelData.h"

#define LED_ON 10
#define LED_OFF 0


    
    /* Check if a position is open relative to another. AKA checking if the
     * pixel at (x + dx, y + dy) is lit already.
     * 
     * @param x     The x position of the origin point.
     * @param y     The y position of the origin point.
     * @param dx    The change in x position. (Should be -1, 0, or 1)
     * @param dy    The change in y position. (Should be -1, 0, or 1)
     */
    uint8_t isOpen(uint8_t x, uint8_t y, int8_t dx, int8_t dy){
        if (getBrightness(x + dx, y + dy) > LED_ON){
            return 0;
        } else {
            return 1;
        }
    }
    
    /* Move a pixel to another position. The pixel at the (xTo, yTo) position
     * will be overwritten with the data from the pixel at the (xFrom, yFrom) 
     * position. Then, the data is erased  at (xFrom, yFtom).
     * 
     * @param xFrom     The x position of the pixel to be moved.
     * @param yFrom     The y position of the pixel to be moved.
     * @param xTo       The new x position of the pixel.
     * @param yTo       The new y position of the pixel.
     */
    void movePixel(uint8_t xFrom, uint8_t yFrom, uint8_t xTo, uint8_t yTo){
        float OrignalVelX = getVelocityX(xFrom, yFrom);
        float OrignalVelY = getVelocityY(xFrom, yFrom);
        float OrignalX = getRawRelativePositionX(xFrom, yFrom);
        float OrignalY = getRawRelativePositionY(xFrom, yFrom);
        float OrignalBright = getBrightness(xFrom, yFrom);
        
        setRawRelativePosition(xTo, yTo, OrignalX - (xTo - xFrom), OrignalY - (yTo - yFrom));
        setMoved(xTo, yTo, 1);
        setVelocity(xTo, yTo, OrignalVelX, OrignalVelY);
        setBrightness(xTo, yTo, OrignalBright);
       
        setRawRelativePosition(xFrom, yFrom, 0, 0);
        setVelocity(xFrom, yFrom, 0.0, 0.0);
        setBrightness(xFrom, yFrom, LED_OFF);
        
    }
    
    /* Try to move a pixel in a direction. If there is already a pixel in the
     * direction to move to, the pixels on either side of that pixel will be 
     * checked. If none of those positions are empty, the pixel will not be moved.
     * 
     * @param x     The x position of the pixel to be moved.
     * @param y     The y position of the pixel to be moved.
     * @param dx    The change in x position. (Should be -1, 0, or 1)
     * @param dy    The change in y position. (Should be -1, 0, or 1)   
     */
    void tryMovePixel(uint8_t x, uint8_t y, int8_t dx, int8_t dy){
        if ((x == 0 && dx < 0) || (x == COLS - 1 && dx > 0)) {
            dx = 0;
            // cannot move, reset raw pos and velocity
            setRawRelativePosition(x, y, 0, getRawRelativePositionY(x, y));
            setVelocity(x, y, 0.0, getVelocityY(x, y));
        }
        
        if ((y == 0 && dy < 0) || (y == ROWS - 1 && dy > 0)) {
            dy = 0;
            // cannot move, reset raw pos and velocity
            setRawRelativePosition(x, y, getRawRelativePositionX(x, y), 0);
            setVelocity(x, y, getVelocityX(x, y), 0.0);
        }
        
        if (isOpen(x, y, 0, 0) == 0 && (dx != 0 || dy != 0)) {
            // pixel present
            if (isOpen(x, y, dx, 0) == 1) {
                movePixel(x, y, x + dx, y);
            } else if (isOpen(x, y, 0, dy) == 1) {
                movePixel(x, y, x, y + dy);
            } else {
                // cannot move, reset velocity
                setVelocity(x, y, 0.0, 0.0);
                setRawRelativePosition(x, y, 0, 0);
            }
            
        }
    }
    
    
    
    /* rounds dx and dy to -1, 0, or 1.
     * 
     * @param d    The change in x or y position. 
     */
    int8_t signint(float d){
        if (d < -0.5){
            return -1;
        } else if (d > 0.5){
            return 1;
        }
        return 0;
    }
    
    
    
    
    /* Apply acceleration to a pixel and possibly move it. The acceleration will
     * be converted into velocity(acceleration * change in time) that is added 
     * to the current velocity of the pixel, and the current velocity will be
     * converted to a change in position (velocity * change in time) which will
     * be added to the current pixel's raw position. If the x or y value of the
     * raw relative position is greater than 1, then the pixel will attempt to
     * move.
     * 
     * dx = vx*dt + (1/2)*ax*dt^2
     * 
     * @param x     The x position of the pixel to apply the acceleration to.
     * @param y     The y position of the pixel to apply acceleration to.
     * @param ax    The x component of the acceleration being applied to the pixel.
     * @param ay    The y component of the acceleration being applied to the pixel.
     * @param dt    The change in time since the last acceleration (in 100 us ticks).
     */
    void applyAcceleration(uint8_t x, uint8_t y, float ax, float ay, unsigned long dt){
        if (isMoved(x, y)) {
            return;
        }
        float OrignalVelX = getVelocityX(x, y);
        float OrignalVelY = getVelocityY(x, y);
        float OrignalX = getRawRelativePositionX(x, y);
        float OrignalY = getRawRelativePositionY(x, y);
        float timeChange = dt / 1000.0f; // convert dt to seconds
           
        
        float CurrentVelX = OrignalVelX + ax * timeChange;
        float CurrentVelY = OrignalVelY + ay * timeChange;
        
        float dx = OrignalVelX * timeChange + 0.5 * ax * timeChange * timeChange;
        float dy = OrignalVelY * timeChange + 0.5 * ay * timeChange * timeChange;
        
        float RawX = OrignalX + dx;
        float RawY = OrignalY + dy;
        
        setRawRelativePosition(x, y, RawX, RawY);
        setVelocity(x, y, CurrentVelX, CurrentVelY);
        
        tryMovePixel(x, y, signint(RawX), signint(RawY));
    }

