/* 
 * File:   lcd.h
 * Author: jpate
 *
 * Created on April 16, 2025, 2:19 PM
 */

#ifndef LCD_H
#define	LCD_H

#ifdef	__cplusplus
extern "C" {
#endif

    void lcd_write_string(const char* str);
    void lcd_set_cursor(uint8_t col, uint8_t row);
    void lcd_init();
    void lcd_clear();
    void delay(int ms);

#ifdef	__cplusplus
}
#endif

#endif	/* LCD_H */

