/* 
 * File:   I2CLib.h
 * Author: Jaden Paterson
 *
 * Created on April 2, 2025, 10:25 AM
 */

#ifndef I2CLIB_H
#define	I2CLIB_H

#ifdef	__cplusplus
extern "C" {
#endif

    // Initialize the I2C1 peripheral with 100k Hz baudrate
    void init_i2c(void);
    
    /* Register a function to be called when data is received from I2C.
    * The function should be in the form: unsigned int receiveEvent(uint8_t, int);
    * The first parameter is the byte received, and the second parameter is the
    * number of bytes remaining in the transmission. The function should return the
    * number of bytes to extend the packet by. If you have a set packet length,
    * always return 0.
    * 
    * @param i2cAddress        The address of the I2C device that this event will fire for.
    * @param functionAddress   The address of the function to be called.
    */
    void register_event(uint8_t i2cAddress, unsigned long address);
    
    /* Send or receive data through I2C
     * 
    * @param address_RW    The least significant bit is for read / not write, and 
    *                      the 7 most significant bits are for the address.
    * @param data[]        The data to be sent. Use 0 for this parameter if reading.
    * @param data_size     The size of the data to be sent or received.
    */
   void transmit_packet(uint8_t address_RW, uint8_t data[], int data_size);


#ifdef	__cplusplus
}
#endif

#endif	/* I2CLIB_H */

