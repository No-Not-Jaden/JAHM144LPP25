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

    /* Write a string to the LCD. You must end it with a null character.
     * 
     * @param str   String to write.
     */
    void lcd_write_string(const char* str);
    
    /* Change the location where text shows up next.
     * 
     * @param col   Column to move the cursor to.
     * @param row   Row to move the cursor to.
     */
    void lcd_set_cursor(uint8_t col, uint8_t row);
    
    /* Initialize the LCD.
     */
    void lcd_init();
    
    /* Clear the contents of the LCD.
     */
    void lcd_clear();

#ifdef	__cplusplus
}
#endif

#endif	/* LCD_H */

