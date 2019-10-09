#ifndef _MPU_H_
#define _MPU_H_

#include <stdbool.h>

bool Mpu_Create(void);
uint8_t Mpu_Read(uint8_t reg);
void Mpu_Write(uint8_t reg, uint8_t data);

#endif // _MPU_H_
