#include "usart.h"
#include "clock.h"
#include "utils/debug.h"
#include "stm32f4xx.h"

#define _BASE(x) ((USART_TypeDef*)x)

usart_t Usart_Create(const usart_options_t* options)
{
    USART_TypeDef* base = options->base;
    pin_options_t tx = {};
    pin_options_t rx = {};
    
    // configure pins
    tx.port = options->tx_port;
    tx.pin = options->tx_pin;
    tx.mode = options->tx_mode;
    Pin_Create(&tx);
    
    rx.port = options->rx_port;
    rx.pin = options->rx_pin;
    rx.mode = options->rx_mode;
    Pin_Create(&rx);
    
    // enable peripheral clock
    Clock_EnablePeripheral(base);
    
    // enable usart
    base->CR1 |= USART_CR1_UE;
    
    // set data length
    if(options->data_len == USART__DATA_LEN__8BITS)
	base->CR1 &= ~USART_CR1_M;
    else
	base->CR1 |= USART_CR1_M;

    // set stop bits
    base->CR2 &= ~USART_CR2_STOP;
    switch(options->stop_bits)
	{
	case USART__STOP_BITS__0P5:
	    base->CR2 |= (1 << 12);
	    break;
	case USART__STOP_BITS__1:
	    break;
	case USART__STOP_BITS__1P5:
	    base->CR2 |= (3 << 12);
	    break;
	case USART__STOP_BITS__2:
	    base->CR2 |= (2 << 12);
	    break;
	}
	
    // clear over8
    base->CR1 &= ~USART_CR1_OVER8;
    
    // set baudrate
    /*
    uint16_t divider = options->baudrate / (16 * 8); //Clock_GetSystemClkFreq());
    uint16_t mantissa = (uint16_t)divider;
    uint8_t fraction = (uint8_t)((divider - mantissa) * 15.0);
    
    if(mantissa >= 4095)
	Debug_Entry(DEBUG__LEVEL_ERROR, "mantissa too high");
    if(fraction >= 15)
	Debug_Entry(DEBUG__LEVEL_ERROR, "fraction too high");
	
    base->BRR = ((mantissa << 4) | fraction);
    */
    // @TODO: system clock may not be the same as peripheral clock
    base->BRR = Clock_GetSystemClkFreq() / options->baudrate;
    
    // enable transmitter
    base->CR1 |= USART_CR1_TE;
    
    return (usart_t)base;
}

void Usart_Transmit(const usart_t handle, const uint8_t c)
{
    // wait until TDR is empty
    while(!(_BASE(handle)->SR & USART_SR_TXE));
    
    // write data and start transmit
    _BASE(handle)->DR = c;
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

