#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx.h"
#include "board_config.h"
#include "clock.h"
#include "pin.h"
#include "usart.h"

void Error(void);
void Blink_Led(uint32_t led, uint32_t delay);
void Delay_Ms(uint32_t count);

int main(void) {
    // enable PWR interface clock
    //REG_SET(RCC->APB1ENR, 1 << 28);

    // enable voltage scale 2 mode
    //REG_CLR(PWR->CR, PWR_CR_VOS);

    // wait for voltage scale to be set
    //while(!(REG_READ(PWR->CSR, 1 << 14)));
    
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    
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

    usart_options_t usart_opts;
    usart_opts.base = USART2;
    usart_opts.baudrate = 9600;
    usart_opts.mode = USART__MODE__USART;
    usart_opts.data_len = USART__DATA_LEN__8BITS;
    usart_opts.stop_bits = USART__STOP_BITS__1;
    usart_opts.tx_port = GPIOA;
    usart_opts.tx_pin = 2;
    usart_opts.tx_mode = PIN__MODE__AF7;
    usart_opts.rx_port = GPIOA;
    usart_opts.rx_pin = 3;
    usart_opts.rx_mode = PIN__MODE__AF7;
    
    usart_t usart = Usart_Create(&usart_opts);
    
    while(1) {
	Usart_Transmit(usart, 'a');
	
	Pin_Set(GPIOD, LED_GREEN);
	Delay_Ms(50);
	Pin_Clear(GPIOD, LED_GREEN);

	Delay_Ms(50);
	
	Pin_Set(GPIOD, LED_RED);
	Delay_Ms(50);
	Pin_Clear(GPIOD, LED_RED);
	
	Delay_Ms(50);
    }
	
    // disable PLL for configuring
    Clock_Disable(CLOCK__PLL);

    // enable HSE clock for PLL source
    Clock_Enable(CLOCK__HSE);

    if(!Clock_SetSystemSource(CLOCK__HSE))
	Error();
	
    // configure PLL
    if(!Clock_ConfigPll(CLOCK__HSE, PLL_M, PLL_N, PLL_P))
	Error();
	
    // enable and set PLL as system source clock
    if(!Clock_SetSystemSource(CLOCK__PLL))
	Error();

    //GPIOD->ODR |= (1 << LED_GREEN);
	
    while(1);
	
    //Blink_Led(LED_GREEN, 100);

    return 0;
}

void Error(void)
{
    Blink_Led(LED_RED, 10);
}

void Blink_Led(uint32_t led, uint32_t delay)
{
    while(1)
	{
	    GPIOD->ODR |= (1 << led);
		
	    Delay_Ms(delay);
	    
	    GPIOD->ODR &= ~(1 << led);
	    Delay_Ms(delay);
	}
}

void Delay_Ms(uint32_t count)
{
	
    uint32_t i;
    uint32_t j;
    //uint32_t loops = 168000000; //(uint32_t)(((float)count / 1000.0) * (float)168000000);
	
    for(i = 0; i < count; i++)
	for(j = 0; j < 10000; j++) //168000
	    ASM VOLATILE("nop");
	
    /*
      uint32_t i;
      uint32_t j;
	
      for(i = 0; i < count; i++)
      for(j = 0; j < 168000; j++)
      ASM VOLATILE("nop");
    */
}
