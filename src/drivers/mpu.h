#ifndef _MPU_H_
#define _MPU_H_

#include <stdbool.h>

bool Mpu_Create(void);
void Mpu_Read(int32_t* x_mg, int32_t* y_mg, int32_t* z_mg);

#endif // _MPU_H_
