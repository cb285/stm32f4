#ifndef _CLOCK_H
#define _CLOCK_H

#include <stdbool.h>

// public clock type
typedef enum {
	CLOCK__HSI,
	CLOCK__HSE,
	CLOCK__PLL,
} clock_t;

// private clock type
typedef struct {
	bool enabled;  		// turned on/off
	void* source; 	    // source clock (only for PLL)
	uint32_t freq; 		// frequency in Hz
} _clock_t;

bool Clock_Enable(clock_t clock);
bool Clock_Disable(clock_t clock);
bool Clock_SetSystemSource(clock_t clock);
uint32_t Clock_GetSystemClkFreq(void);
bool Clock_SetPllSource(clock_t clock);
bool Clock_ConfigPll(clock_t source_clock, uint32_t pllm, uint32_t plln, uint32_t pllp);
void Clock_EnablePeripheral(void* base);

#endif // _CLOCK_H
