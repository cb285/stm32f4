#include "mpu.h"
#include "drivers/spi.h"

static spi_t _spi = NULL;

bool Mpu_Create(void) {
    spi_options_t spi_opts = {};
    spi_opts.base = SPI1;
    spi_opts.baudrate = 10000000;
    spi_opts.data_len = SPI__DATA_LEN__8BITS;
    spi_opts.polarity = SPI__POLARITY__IDLE_LOW;
    spi_opts.phase = SPI__PHASE__RISING;
    spi_opts.mosi_port = GPIOA;
    spi_opts.mosi_pin = 7;
    spi_opts.mosi_mode = PIN__MODE__AF5;
    spi_opts.miso_port = GPIOA;
    spi_opts.miso_pin = 6;
    spi_opts.miso_mode = PIN__MODE__AF5;
    spi_opts.sck_port = GPIOA;
    spi_opts.sck_pin = 5;
    spi_opts.sck_mode = PIN__MODE__AF5;
    spi_opts.cs_port = GPIOE;
    spi_opts.cs_pin = 3;
    spi_opts.cs_mode = PIN__MODE__OUTPUT_HIGH; // manual CS control

    _spi = Spi_Create(&spi_opts);

    if(_spi)
	return true;
    return false;
}

uint8_t Mpu_Read(uint8_t reg) {
    
    if(!_spi)
	return 0;
    
    reg |= (1 << 7);
    uint8_t rx;
    Spi_Transfer(_spi, &reg, NULL, 1, NULL, &rx, 1);
    return rx;
}

void Mpu_Write(uint8_t reg, uint8_t data) {
    
    if(!_spi)
	return;
    
    Spi_Transfer(_spi, &reg, NULL, 1, &data, NULL, 1);
}
