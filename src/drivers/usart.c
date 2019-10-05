#include "usart.h"
#include "clock.h"
#include "utils/debug.h"
#include "stm32f4xx.h"

#define _BASE(x) ((USART_TypeDef*)x)

usart_t Usart_Create(const usart_options_t* options)
{
    USART_TypeDef* base = options->base;
	
    // enable peripheral clock
    if(base == USART1)
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    else if(base == USART2)
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    else if(base == USART3)
	RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    else if(base == UART4)
	RCC->APB1ENR |= RCC_APB1ENR_UART4EN;
    else if(base == UART5)
	RCC->APB1ENR |= RCC_APB1ENR_UART5EN;
    else if(base == USART6)
	RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
	
    // enable usart
    base->CR1 |= USART_CR1_UE;

    // set data length
    if(options->data_len == USART__DATA_LEN_8BITS)
	base->CR1 &= ~USART_CR1_M;
    else
	base->CR1 |= USART_CR1_M;

    // set stop bits
    base->CR2 &= ~USART_CR2_STOP;
    switch(options->stop_bits)
	{
	case USART__STOP_BITS_0P5:
	    base->CR2 |= (1 << 12);
	    break;
	case USART__STOP_BITS_1:
	    break;
	case USART__STOP_BITS_1P5:
	    base->CR2 |= (3 << 12);
	    break;
	case USART__STOP_BITS_2:
	    base->CR2 |= (2 << 12);
	    break;
	}
	
    // clear over8
    base->CR1 &= ~USART_CR1_OVER8;
    
    // set baudrate
    uint16_t divider = options->baudrate / (16 * Clock_GetSystemClkFreq());
    uint16_t mantissa = (uint16_t)divider;
    uint8_t fraction = (uint8_t)((divider - mantissa) * 15.0);
    
    if(mantissa >= 4095)
	Debug_Entry(DEBUG__LEVEL_ERROR, "mantissa too high");
    if(fraction >= 15)
	Debug_Entry(DEBUG__LEVEL_ERROR, "fraction too high");
    
    // enable transmitter
    base->CR1 |= USART_CR1_TE;
    
    return (usart_t)base;
}

void Usart_Transmit(const usart_t handle, const uint8_t c)
{
    // write data and start transmit
    _BASE(handle)->DR = c;
    
    // wait until transmit is complete
    while(!(_BASE(handle)->SR & USART_SR_TC));
}

/*
  void Usart_Destroy()
  {
  // disable peripheral clock
  switch(base)
  {
  case USART1:
  BITS_CLR(RCC->APB2ENR, RCC_APB2ENR_USART1EN);
  case USART2:
  BITS_CLR(RCC->APB1ENR, RCC_APB1ENR_USART2EN);
  case USART3:
  BITS_CLR(RCC->APB1ENR, RCC_APB1ENR_USART3EN);
  case UART4:
  BITS_CLR(RCC->APB1ENR, RCC_APB1ENR_UART4EN);
  case UART5:
  BITS_CLR(RCC->APB1ENR, RCC_APB1ENR_UART5EN);
  case USART6:
  BITS_CLR(RCC->APB2ENR, RCC_APB2ENR_USART6EN);
  }
  }
*/

