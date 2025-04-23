/*
 * File:   PositionCalculator.c
 * Author: Christian
 *
 * Created on April 21, 2025, 11:01 AM
 */


#include "xc.h"
#include "PixelData.h"

#define LED_ON 40
#define LED_OFF 0


    
    /* Check if a position is open relative to another. AKA checking if the
     * pixel at (x + dx, y + dy) is lit already.
     * 
     * @param x     The x position of the origin point.
     * @param y     The y position of the origin point.
     * @param dx    The change in x position. (Should be -1, 0, or 1)
     * @param dy    The change in y position. (Should be -1, 0, or 1)
     */
    uint8_t isOpen(uint8_t x, uint8_t y, int dx, int dy){
        if (getBrightness(x + dx, y + dy) == LED_ON){
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
        
         if (xFrom != xTo && yFrom != yTo){
            setRawRelativePosition(xTo, yTo, 0.0, 0.0);
        } else if (xFrom != xTo){
            setRawRelativePosition(xTo, yTo, 0.0, OrignalY);
        } else if (yFrom != yTo){
            setRawRelativePosition(xTo, yTo, OrignalX, 0.0);
        } else {
            setRawRelativePosition(xTo, yTo, OrignalX, OrignalY);
        }
               
        setVelocity(xTo, yTo, OrignalVelX, OrignalVelY);
        setBrightness(xTo, yTo, OrignalBright);
       
        setRawRelativePosition(xFrom, yFrom, 0, 0);
        setVelocity(xFrom, yFrom, 0.0, 0.0);
        setBrightness(xTo, yTo, LED_OFF);
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
    void tryMovePixel(uint8_t x, uint8_t y, int dx, int dy){
        if (isOpen(x, y, dx, dy) == 1){
            movePixel(x, y, x + dx, y + dy);            
        }
    }
    
    
    
    /* rounds dx and dy to -1, 0, or 1.
     * 
     * @param d    The change in x or y position. 
     */
    uint8_t round(float d){
        if (d < -1){
            return -1;
        } else if (-1 <= d && d <= 1){
            return 0;
        } else if (d > 1){
            return 1;
        }
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
     * @param dt    The change in time since the last acceleration (in milliseconds).
     */
    void applyAcceleration(uint8_t x, uint8_t y, float ax, float ay, unsigned int dt){
        float OrignalVelX = getVelocityX(x, y);
        float OrignalVelY = getVelocityY(x, y);
        float OrignalX = getRawRelativePositionX(x, y);
        float OrignalY = getRawRelativePositionY(x, y);
        
        float AccelX = OrignalVelX * dt;
        float AccelY = OrignalVelY * dt;          
        
        float CurrentVelX = OrignalVelX + AccelX * dt;
        float CurrentVelY = OrignalVelY + AccelY * dt;
        
        float dx = OrignalVelX * dt + 0.5 * AccelX * dt * dt;
        float dy = OrignalVelY * dt + 0.5 * AccelY * dt * dt;
        
        float RawX = OrignalX + dx;
        float RawY = OrignalY + dx;
        
        setRawRelativePosition(x, y, RawX, RawY);
        setVelocity(x, y, CurrentVelX, CurrentVelY);
        
        if ( (RawX > 1 || RawX < -1) || (RawY > 1 || RawY < -1) ) {
            tryMovePixel(x, y, round(dx), round(dy));
        }
//        if (RawX > 1 && RawY > 1){
//            tryMovePixel(x, y, dx, dy);    
//        } else if (RawX < -1 && RawY > 1){
//            tryMovePixel(x, y, dx, dy);
//        } else if (RawX > 1 && RawY < -1){
//            tryMovePixel(x, y, dx, dy);
//        } else if (RawX < -1 && RawY < -1){
//            tryMovePixel(x, y, dx, dy);
//        } else if (RawX < -1){
//            tryMovePixel(x, y, dx, dy);
//        } else if (RawX > 1){
//            tryMovePixel(x, y, dx, dy);
//        } else if (RawY > 1){
//            tryMovePixel(x, y, dx, dy);
//        } else if (RawY < -1){
//            tryMovePixel(x, y, dx, dy);
//        }
    }

