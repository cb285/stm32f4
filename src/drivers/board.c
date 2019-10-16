#include "board.h"
#include "board_config.h"
#include "drivers/clock.h"
#include "cmsis.h"

static void _InitClocks(void) {
    // enable HSE clock for PLL source
    Clock_Enable(CLOCK__HSE);
    
    // disable PLL for configuring
    Clock_Disable(CLOCK__PLL);
    
    // configure PLL
    Clock_ConfigPll(CLOCK__HSE, PLL_M, PLL_N, PLL_P);
    
    // enable PLL and set as system source clock
    Clock_SetSystemSource(CLOCK__PLL);
}

void Board_Init(void) {

    // enable FPU
    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */

    // init clocks
    _InitClocks();
    
}
