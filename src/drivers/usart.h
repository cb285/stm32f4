#ifndef _USART_H
#define _USART_H

// public handle type
typedef void* usart_t;

typedef enum {
	USART__DATA_LEN_8BITS,
	USART__DATA_LEN_9BITS
} usart_data_len_t;

typedef enum {
	USART__STOP_BITS_0P5, // fractions are not available for UARTS
	USART__STOP_BITS_1,
	USART__STOP_BITS_1P5,
	USART__STOP_BITS_2
} usart_stop_bits_t;

typedef enum {
	USART__MODE_USART,
	USART__MODE_SPI,
	USART__MODE_SMARTCARD,
	USART__MODE_LIN,
	USART__MODE_IRDA
} usart_mode_t;

typedef struct {
	void* base;
	uint32_t baudrate;
	usart_mode_t mode;
	usart_data_len_t data_len;
	usart_stop_bits_t stop_bits;
} usart_options_t;

usart_t Usart_Create(const usart_options_t* config);
void Usart_Transmit(const usart_t handle, const uint8_t c);

#endif // _USART_H
