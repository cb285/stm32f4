#include "spi.h"
#include "drivers/pin.h"
#include "drivers/clock.h"
#include "utils/log.h"
#include "utils/heap.h"

#define _HANDLE(x) ((_spi_t*)x)
#define _BASE(x) (_HANDLE(x)->base)

// private handle
typedef struct {
    SPI_TypeDef* base;
    pin_port_t cs_port;
    pin_num_t cs_pin;
    bool active;
} _spi_t;

spi_t Spi_Create(const spi_options_t* options) {
    SPI_TypeDef* base = options->base;

    pin_options_t mosi = {};
    pin_options_t miso = {};
    pin_options_t sck = {};
    pin_options_t cs = {};
    
    // allocate private handle
    _spi_t* handle = Malloc(sizeof(_spi_t));
    
    // copy base address
    handle->base = options->base;
    
    // check if doing manual CS control
    if(options->cs_mode == PIN__MODE__OUTPUT_LOW || options->cs_mode == PIN__MODE__OUTPUT_HIGH) {
	handle->cs_port = options->cs_port;
	handle->cs_pin = options->cs_pin;

	cs.port = options->cs_port;
	cs.pin = options->cs_pin;
	cs.mode = PIN__MODE__OUTPUT_HIGH;
	Pin_Create(&cs);
    }
    else {
	handle->cs_port = 0;
	handle->cs_pin = 0;

	cs.port = options->cs_port;
	cs.pin = options->cs_pin;
	cs.mode = options->cs_mode;
	Pin_Create(&cs);
    }
    
    // configure pins
    mosi.port = options->mosi_port;
    mosi.pin = options->mosi_pin;
    mosi.mode = options->mosi_mode;
    mosi.attr = PIN__ATTR__SPEED_HIGHEST;
    Pin_Create(&mosi);
    
    miso.port = options->miso_port;
    miso.pin = options->miso_pin;
    miso.mode = options->miso_mode;
    miso.attr = PIN__ATTR__SPEED_HIGHEST;
    Pin_Create(&miso);
    
    sck.port = options->sck_port;
    sck.pin = options->sck_pin;
    sck.mode = options->sck_mode;
    sck.attr = PIN__ATTR__SPEED_HIGHEST;
    Pin_Create(&sck);
    
    // enable peripheral clock
    Clock_EnablePeripheral(base);
    
    // reset CR1
    base->CR1 = 0;

    // reset CR2
    base->CR2 = 0;

    // enable peripheral
    base->CR1 |= SPI_CR1_SPE;

    // enable software CS control if doing manual control
    if(handle->cs_port)
	base->CR1 |= (SPI_CR1_SSM | SPI_CR1_SSI);
    
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

    // clear active flag
    handle->active = false;
    
    return handle;
}

void Spi_Transmit(const spi_t handle, uint8_t c) {

    // do manual control if not in active transfer
    if(_HANDLE(handle)->cs_port && !_HANDLE(handle)->active)
	Pin_Clear(_HANDLE(handle)->cs_port, _HANDLE(handle)->cs_pin);
    
    while(!(_BASE(handle)->SR & SPI_SR_TXE));
    _BASE(handle)->DR = c;
    
    // do manual control if not in active transfer
    if(_HANDLE(handle)->cs_port && !_HANDLE(handle)->active)
	Pin_Set(_HANDLE(handle)->cs_port, _HANDLE(handle)->cs_pin);
}

uint8_t Spi_Receive(const spi_t handle) {

    // do manual control if not in active transfer
    if(_HANDLE(handle)->cs_port && !_HANDLE(handle)->active)
	Pin_Clear(_HANDLE(handle)->cs_port, _HANDLE(handle)->cs_pin);
    
    while(!(_BASE(handle)->SR & SPI_SR_RXNE));

    // do manual control if not in active transfer
    if(_HANDLE(handle)->cs_port && !_HANDLE(handle)->active)
	Pin_Set(_HANDLE(handle)->cs_port, _HANDLE(handle)->cs_pin);
    
    return _BASE(handle)->DR;
}

void Spi_Transfer(const spi_t handle, const uint8_t* tx1, uint8_t* rx1, uint32_t len1, const uint8_t* tx2, uint8_t* rx2, uint32_t len2) {
    uint32_t i;

    // manual CS control
    if(_HANDLE(handle)->cs_port) {
	Pin_Clear(_HANDLE(handle)->cs_port, _HANDLE(handle)->cs_pin);
	_HANDLE(handle)->active = true;
    }
    
    for(i = 0; i < len1; i++) {
	if(tx1)
	    Spi_Transmit(handle, tx1[i]);
	else
	    Spi_Transmit(handle, 0);
	if(rx1)
	    rx1[i] = Spi_Receive(handle);
	else
	    Spi_Receive(handle);
	i++;
    }
    
    for(i = 0; i < len2; i++) {
	if(tx2)
	    Spi_Transmit(handle, tx2[i]);
	else
	    Spi_Transmit(handle, 0);
	if(rx2)
	    rx2[i] = Spi_Receive(handle);
	else
	    Spi_Receive(handle);
	i++;
    }

    // manual CS control
    if(_HANDLE(handle)->cs_port) {
	Pin_Set(_HANDLE(handle)->cs_port, _HANDLE(handle)->cs_pin);
	_HANDLE(handle)->active = false;
    }
}
