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

    int16_t x, y, z;
    
    while(1) {
	Mpu_Read(&x, &y, &z);
	//Debug_Log(DEBUG__LEVEL__INFO, "%d.%d, %d.%d, %d.%d", x_int / 100, x_int % 100, y_int / 100, y_int % 100, z_int / 100, z_int % 100);
	Debug_Log(DEBUG__LEVEL__INFO, "%d, %d, %d", x, y, z);
	/*
	Pin_Set(GPIOD, LED_GREEN);
	Delay_Ms(50);
	Pin_Clear(GPIOD, LED_GREEN);

	Delay_Ms(50);
	
	Pin_Set(GPIOD, LED_RED);
	Delay_Ms(50);
	Pin_Clear(GPIOD, LED_RED);
	
	Delay_Ms(50);
	*/
    }
	
    return 0;
}
