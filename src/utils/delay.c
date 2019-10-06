#include "delay.h"
#include "drivers/clock.h"

void Delay_Us(uint32_t delay)
{
    uint32_t i;
    uint32_t j;
    uint32_t freq = Clock_GetFreq(CLOCK__SYSCLK);
    
    for(i = 0; i < delay; i++)
	for(j = 0; j < (freq / 20000000); j++)
	    ASM VOLATILE("nop");
}

void Delay_Ms(uint32_t delay) {
    while(delay--)
	Delay_Us(1000);
}
