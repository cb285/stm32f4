#ifndef _USART_H
#define _USART_H

#include "drivers/pin.h"

// public handle type
typedef void* usart_t;

typedef enum {
    USART__DATA_LEN__8BITS,
    USART__DATA_LEN__9BITS
} usart_data_len_t;

typedef enum {
    USART__STOP_BITS__0P5, // fractions are not available for UARTS
    USART__STOP_BITS__1,
    USART__STOP_BITS__1P5,
    USART__STOP_BITS__2
} usart_stop_bits_t;

typedef enum {
    USART__MODE__USART,
    USART__MODE__SPI,
    USART__MODE__SMARTCARD,
    USART__MODE__LIN,
    USART__MODE__IRDA
} usart_mode_t;

typedef struct {
    void*             base;
    uint32_t          baudrate;
    usart_mode_t      mode;
    usart_data_len_t  data_len;
    usart_stop_bits_t stop_bits;
    pin_port_t        tx_port;
    pin_num_t         tx_pin;
    pin_mode_t        tx_mode;
    pin_port_t        rx_port;
    pin_num_t         rx_pin;
    pin_mode_t        rx_mode;
} usart_options_t;

usart_t Usart_Create(const usart_options_t* config);
void Usart_Transmit(const usart_t handle, const uint8_t c);
void Usart_WriteString(const usart_t handle, const char* str);

#endif // _USART_H
