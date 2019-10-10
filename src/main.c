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
#include <stdio.h>
#include <stdlib.h>

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

    double x, y, z;
    int x_int, y_int, z_int;
    char x_str[5];
    char y_str[5];
    char z_str[5];
    
    while(1) {
	Mpu_Read(&x, &y, &z);

	x_int = x * 100;
	y_int = y * 100;
	z_int = z * 100;
	
	if(x_int < 0)
	    sprintf(x_str, "-%d.%02d", abs(x_int) / 100, abs(x_int) % 100);
	else
	    sprintf(x_str, "%d.%02d", x_int / 100, x_int % 100);
	if(y_int < 0)
	    sprintf(y_str, "-%d.%02d", abs(y_int) / 100, abs(y_int) % 100);
	else
	    sprintf(y_str, "%d.%02d", y_int / 100, y_int % 100);
	if(z_int < 0)
	    sprintf(z_str, "-%d.%02d", abs(z_int) / 100, abs(z_int) % 100);
	else
	    sprintf(z_str, "%d.%02d", z_int / 100, z_int % 100);
	    
	Debug_Log(DEBUG__LEVEL__INFO, "%s, %s, %s", x_str, y_str, z_str);
	
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
