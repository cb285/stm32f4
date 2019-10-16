#include "delay.h"
#include "drivers/timer.h"
#include <stdbool.h>

static uint32_t _initialized = false;
static uint32_t _freq;

void Delay_Us(uint32_t delay) {
    uint64_t target;
    
    if(!_initialized) {
	// create timer
	Timer_Create();

	// get frequency
	_freq = Timer_GetFreq();

	_initialized = true;
    }
    
    // calculate target counter value
    target = Timer_Read() + (delay * (_freq / 1000000.0));
    
    // wait to reach target
    while(Timer_Read() < target);
}

void Delay_Ms(uint32_t delay) {
    while(delay--)
	Delay_Us(1000);
}
