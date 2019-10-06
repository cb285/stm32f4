#include "board.h"
#include "board_config.h"
#include "drivers/clock.h"

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
    _InitClocks();
}
