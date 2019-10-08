#ifndef _SPI_H_
#define _SPI_H_

#include "drivers/pin.h"

// public handle
typedef void* spi_t;

typedef enum {
    SPI__DATA_LEN__8BITS,
    SPI__DATA_LEN__16BITS
} spi_data_len_t;

typedef enum {
    SPI__POLARITY__IDLE_HIGH,
    SPI__POLARITY__IDLE_LOW
} spi_polarity_t;

typedef enum {
    SPI__PHASE__RISING,
    SPI__PHASE__FALLING
} spi_phase_t;

typedef struct {
    void* base;
    uint32_t baudrate;
    spi_data_len_t data_len;    
    spi_polarity_t polarity;
    spi_phase_t phase;
    // MOSI
    pin_port_t mosi_port;
    pin_num_t mosi_pin;
    pin_mode_t mosi_mode;
    // MISO
    pin_port_t miso_port;
    pin_num_t miso_pin;
    pin_mode_t miso_mode;
    // SCK
    pin_port_t sck_port;
    pin_num_t sck_pin;
    pin_mode_t sck_mode;
    // CS
    pin_port_t cs_port;
    pin_num_t cs_pin;
    pin_mode_t cs_mode;
} spi_options_t;

spi_t Spi_Create(const spi_options_t* options);
void Spi_Transmit(const spi_t handle, uint8_t c);
uint8_t Spi_Receive(const spi_t handle);

#endif // _SPI_H_
