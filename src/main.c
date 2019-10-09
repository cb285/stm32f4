#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx.h"
#include "board_config.h"
#include "drivers/board.h"
#include "drivers/clock.h"
#include "drivers/pin.h"
#include "utils/debug.h"
#include "utils/delay.h"
#include "drivers/mpu.h"

int main(void) {
    /*
    // enable PWR interface clock
    RCC->APB1ENR |= 1 << 28;

    // enable voltage scale 2 mode
    PWR->CR &= ~PWR_CR_VOS;

    // wait for voltage scale to be set
    while(!(PWR->CSR & (1 << 14)));
    */
    
    // init clock
    Board_Init();
    
    pin_options_t green;
    green.port = GPIOD;
    green.pin = LED_GREEN;
    green.mode = PIN__MODE__OUTPUT_LOW;
    
    pin_options_t red;
    red.port = GPIOD;
    red.pin = LED_RED;
    red.mode = PIN__MODE__OUTPUT_LOW;
    
    Pin_Create(&green);
    Pin_Create(&red);
    
    Debug_Create();
    
    Mpu_Create();
    
    while(1) {
	Debug_Log(DEBUG__LEVEL__INFO, "WHO_AM_I = 0x%x", Mpu_Read(0xf));
	
	Pin_Set(GPIOD, LED_GREEN);
	Delay_Ms(50);
	Pin_Clear(GPIOD, LED_GREEN);

	Delay_Ms(50);
	
	Pin_Set(GPIOD, LED_RED);
	Delay_Ms(50);
	Pin_Clear(GPIOD, LED_RED);
	
	Delay_Ms(50);
    }
	
    return 0;
}
