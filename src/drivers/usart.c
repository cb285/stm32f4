#include "usart.h"
#include "clock.h"
#include "utils/debug.h"
#include "stm32f4xx.h"

#define _BASE(x) ((USART_TypeDef*)x)

usart_t Usart_Create(const usart_options_t* options) {
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
    base->BRR = Clock_GetPeripheralFreq(base) / options->baudrate;
    
    // enable transmitter and receiver
    base->CR1 |= (USART_CR1_TE | USART_CR1_RE);
    
    return (usart_t)base;
}

// @TODO timeout
uint8_t Usart_Receive(const usart_t handle, uint32_t timeout) {
    // wait until data ready
    while(!(_BASE(handle)->SR & USART_SR_RXNE));

    // return data
    return _BASE(handle)->DR;
}

void Usart_Transmit(const usart_t handle, const uint8_t c) {
    // wait until TDR is empty
    while(!(_BASE(handle)->SR & USART_SR_TXE));
    
    // write data and start transmit
    _BASE(handle)->DR = c;
}

// @TODO timeout
uint32_t Usart_Read(const usart_t handle, void* data, uint32_t max_len, uint32_t timeout) {
    uint32_t i;
    for(i = 0; i < max_len; i++)
	((uint8_t*)data)[i] = Usart_Receive(handle, timeout);
    return i;
}

void Usart_Write(const usart_t handle, const void* data, uint32_t len) {
    for(uint32_t i = 0; i < len; i++)
	Usart_Transmit(handle, ((uint8_t*)data)[i]);
}

void Usart_WriteString(const usart_t handle, const char* str) {
    uint32_t i = 0;
    while(str[i] != '\0')
	Usart_Transmit(handle, str[i++]);
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

