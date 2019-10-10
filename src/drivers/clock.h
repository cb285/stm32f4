#ifndef _CLOCK_H
#define _CLOCK_H

#include <stdbool.h>

// public clock type
typedef enum {
	CLOCK__HSI,
	CLOCK__HSE,
	CLOCK__PLL,
	CLOCK__SYSCLK
} mclock_t;

// private clock type
typedef struct {
	bool enabled;  	// turned on/off
	void* source; 	// source clock (only for PLL)
	uint32_t freq; 	// frequency in Hz
} _clock_t;

bool Clock_Enable(mclock_t clock);
bool Clock_Disable(mclock_t clock);
bool Clock_SetSystemSource(mclock_t clock);
uint32_t Clock_GetFreq(mclock_t clock);
bool Clock_SetPllSource(mclock_t clock);
bool Clock_ConfigPll(mclock_t source_clock, uint32_t pllm, uint32_t plln, uint32_t pllp);
void Clock_EnablePeripheral(const void* base);
uint32_t Clock_GetPeripheralFreq(const void* base);

#endif // _CLOCK_H
