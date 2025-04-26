/* 
 * File:   BNO085.h
 * Author: Jaden Paterson
 *
 * Created on April 3, 2025, 9:37 AM
 */

#ifndef BNO085_H
#define	BNO085_H

#ifdef	__cplusplus
extern "C" {
#endif
    
    // Structure of the gravity vector
    typedef struct {
        float x;
        float y;
        float z;
        unsigned long deltaTime;    // total time this object encompasses
        unsigned int average_count; // how many averages in this object
    } GravityVector;

    /* Initialize the bno085 and start receiving reports
     */
    void bno085_init();
    /* Get the gravity vector from the device.
     * 
     * @param out   The gravity vector
     */
    void getGravityVector(GravityVector* out);
    
    void getAccVector(GravityVector* out);


#ifdef	__cplusplus
}
#endif

#endif	/* BNO085_H */

