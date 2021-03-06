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

static inline void _WriteDr(const spi_t handle, const void* ptr, uint32_t index) {
    if(ptr)
	_BASE(handle)->DR = ((uint8_t*)ptr)[index];
    else
	_BASE(handle)->DR = 0;
}

static inline void _ReadDr(const spi_t handle, const void* ptr, uint32_t index) {
    if(ptr)
	((uint8_t*)ptr)[index] = _BASE(handle)->DR;
    else
	(void)_BASE(handle)->DR;
}

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

void Spi_Destroy(spi_t* handle_ptr) {

    // check valid
    if(!handle_ptr || !*handle_ptr)
	return;
    
    // free private handle
    Free(*handle_ptr);
    
    // set handle to NULL to avoid reuse
    *handle_ptr = NULL;
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

void Spi_Transfer(const spi_t handle, const void* tx1, void* rx1, uint32_t len1, const void* tx2, void* rx2, uint32_t len2) {
    uint32_t idx_tx = 0;
    uint32_t idx_rx = 0;
    
    // manual CS control
    if(_HANDLE(handle)->cs_port)
	Pin_Clear(_HANDLE(handle)->cs_port, _HANDLE(handle)->cs_pin);
    
    // set active flag
    _HANDLE(handle)->active = true;
        
    // send first byte
    if(idx_tx < len1) {
	while(!(_BASE(handle)->SR & SPI_SR_TXE));
	_WriteDr(handle, tx1, idx_tx++);
    }
    
    while(idx_tx < len1) {	
	// load next byte
	while(!(_BASE(handle)->SR & SPI_SR_TXE));
	_WriteDr(handle, tx1, idx_tx++);

	// receive byte
	while(!(_BASE(handle)->SR & SPI_SR_RXNE));
	_ReadDr(handle, rx1, idx_rx++);
    }

    // send first byte of tx2
    idx_tx = 0;
    if(idx_tx < len2) {
	while(!(_BASE(handle)->SR & SPI_SR_TXE));
	_WriteDr(handle, tx2, idx_tx++);
    }

    // receive last byte of rx1
    if(idx_rx < len1) {
	while(!(_BASE(handle)->SR & SPI_SR_RXNE));
	_ReadDr(handle, rx1, idx_rx++);
    }
    
    // receiving rx2 now
    idx_rx = 0;

    while(idx_tx < len2) {	
	// load next byte
	while(!(_BASE(handle)->SR & SPI_SR_TXE));
	_WriteDr(handle, tx2, idx_tx++);
	
	// receive byte
	while(!(_BASE(handle)->SR & SPI_SR_RXNE));
	_ReadDr(handle, rx2, idx_rx++);
    }

    // receive last byte of rx2
    if(idx_rx < len2) {
	while(!(_BASE(handle)->SR & SPI_SR_RXNE));
	_ReadDr(handle, rx2, idx_rx++);
    }
    
    // manual CS control
    if(_HANDLE(handle)->cs_port) {
	Pin_Set(_HANDLE(handle)->cs_port, _HANDLE(handle)->cs_pin);
	_HANDLE(handle)->active = false;
    }
}
