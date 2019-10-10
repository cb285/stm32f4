#ifndef _MPU_H_
#define _MPU_H_

#include <stdbool.h>

bool Mpu_Create(void);
void Mpu_Read(int16_t* x, int16_t* y, int16_t* z);

#endif // _MPU_H_
