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
    
    typedef struct {
        float x;
        float y;
        float z;
    } Vector3d;

void bno085_init();
void getGravityVector(Vector3d* out);


#ifdef	__cplusplus
}
#endif

#endif	/* BNO085_H */

