#include "timer.h"
#include "stm32f4xx.h"
#include "drivers/clock.h"
#include <stdbool.h>
#include "drivers/irq.h"

#define _TIMER TIM2
#define _IRQ TIM2_IRQn

static bool _initialized = false;
static uint32_t _freq = 0;
static uint32_t _upper32 = 0;

void TIM2_IRQHandler(void) {
    
    // check for overflow
    if(_TIMER->SR & TIM_SR_UIF) {
	
	// increment upper bits
	_upper32++;
    }
    
    // clear interrupt
    _TIMER->SR = 0;
}

void Timer_Create(void) {

    if(_initialized)
	return;
    
    // enable peripheral clock
    Clock_EnablePeripheral(_TIMER);
    
    // reset CR1
    _TIMER->CR1 = 0;
    
    // set auto reload value to max
    _TIMER->ARR = 0xffffffff;

    // set prescaler to 0 (divider of 1)
    _TIMER->PSC = 0;
	
    // clear status
    _TIMER->SR = 0;

    // get timer frequency
    _freq = Clock_GetPeripheralFreq(_TIMER);
    
    // start counter
    _TIMER->CR1 |= TIM_CR1_CEN;

    // enable update interrupt
    _TIMER->DIER = TIM_DIER_UIE;
    Irq_SetPriority(_IRQ, Irq_GetLowestPriority());
    Irq_Enable(_IRQ);
    
    _initialized = true;
}

uint32_t Timer_GetFreq(void) {
    if(!_initialized)
	return 0;

    return _freq;
}

uint64_t Timer_Read(void) {
    if(!_initialized)
	return 0;
    
    return (((uint64_t)_upper32 << 32) | _TIMER->CNT);
}

