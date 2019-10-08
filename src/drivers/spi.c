#include "spi.h"
#include "drivers/pin.h"
#include "drivers/clock.h"
#include "utils/log.h"

#define _BASE(x) ((SPI_TypeDef*)x)

spi_t Spi_Create(const spi_options_t* options) {
    SPI_TypeDef* base = options->base;

    pin_options_t mosi = {};
    pin_options_t miso = {};
    pin_options_t sck = {};
    pin_options_t cs = {};

    // configure pins
    mosi.port = options->mosi_port;
    mosi.pin = options->mosi_pin;
    mosi.mode = options->mosi_mode;
    Pin_Create(&mosi);

    miso.port = options->miso_port;
    miso.pin = options->miso_pin;
    miso.mode = options->miso_mode;
    Pin_Create(&miso);

    sck.port = options->sck_port;
    sck.pin = options->sck_pin;
    sck.mode = options->sck_mode;
    Pin_Create(&sck);

    cs.port = options->cs_port;
    cs.pin = options->cs_pin;
    cs.mode = options->cs_mode;
    Pin_Create(&cs);

    // enable peripheral clock
    Clock_EnablePeripheral(base);

    // reset CR1
    base->CR1 = 0;

    // reset CR2
    base->CR2 = 0;
    
    // enable peripheral
    base->CR1 |= SPI_CR1_SPE;

    // enable CS (must be done before enabling master mode)
    base->CR2 |= SPI_CR2_SSOE;
    
    // set master mode
    base->CR1 |= SPI_CR1_MSTR;

    // set baud rate
    base->CR1 |= (Log2(Clock_GetPeripheralFreq(base) / options->baudrate) - 1) << 3;
    
    // set polarity
    if(options->polarity == SPI__POLARITY__IDLE_HIGH)
	base->CR1 |= SPI_CR1_CPOL;
    
    // set phase
    if(options->phase == SPI__PHASE__FALLING)
	base->CR1 |= SPI_CR1_CPHA;
    
    // set data size
    if(options->data_len == SPI__DATA_LEN__16BITS)
	base->CR1 |= SPI_CR1_DFF;
    
    return (spi_t)base;
}

void Spi_Transmit(const spi_t handle, uint8_t c) {
    while(!(_BASE(handle)->SR & SPI_SR_TXE));
    _BASE(handle)->DR = c;
}

uint8_t Spi_Receive(const spi_t handle) {
    while(!(_BASE(handle)->SR & SPI_SR_RXNE));
    return _BASE(handle)->DR;
}
