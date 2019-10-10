#ifndef _MPU_H_
#define _MPU_H_

#include <stdbool.h>

bool Mpu_Create(void);
void Mpu_Read(double* x, double* y, double* z);

#endif // _MPU_H_
