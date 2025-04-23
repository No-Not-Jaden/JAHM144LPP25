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

    void led_init(void);
    void led_write(uint8_t row, uint8_t column, uint8_t brightness);
    void write_all ();

#ifdef	__cplusplus
}
#endif

#endif	/* LED_144_LIB_H */

