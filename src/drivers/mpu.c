#include "mpu.h"
#include "drivers/spi.h"

// to m/s^2
#define _CONVERT(x) (x * (9.80665 * 2.0 / 32767.0))

// registers
#define _WHO_AM_I                0xf
#define _WHO_AM_I__VAL           0x3f
#define _CTRL_REG6               0x25
#define _CTRL_REG6__BOOT         (1 << 7)
#define _CTRL_REG6__FIFO_EN      (1 << 6)
#define _CTRL_REG6__ADD_INC      (1 << 4)
#define _STATUS                  0x27
#define _STATUS__ZYXDA           (1 << 3)
#define _OUT_X                   0x28
#define _OUT_Y                   0x2a
#define _OUT_Z                   0x2c
#define _CTRL_REG4               0x20
#define _CTRL_REG4__XEN          (1 << 0)
#define _CTRL_REG4__YEN          (1 << 1)
#define _CTRL_REG4__ZEN          (1 << 2)
#define _CTRL_REG4__BDR          (1 << 3)
#define _CTRL_REG4__ODR__Pos     4
#define _CTRL_REG4__ODR__0HZ     (0x0 << _CTRL_REG4__ODR__Pos)
#define _CTRL_REG4__ODR__3P125HZ (0x1 << _CTRL_REG4__ODR__Pos)
#define _CTRL_REG4__ODR__6P25HZ  (0x2 << _CTRL_REG4__ODR__Pos)
#define _CTRL_REG4__ODR__12P5HZ  (0x3 << _CTRL_REG4__ODR__Pos)
#define _CTRL_REG4__ODR__25HZ    (0x4 << _CTRL_REG4__ODR__Pos)
#define _CTRL_REG4__ODR__50HZ    (0x5 << _CTRL_REG4__ODR__Pos)
#define _CTRL_REG4__ODR__100HZ   (0x6 << _CTRL_REG4__ODR__Pos)
#define _CTRL_REG4__ODR__400HZ   (0x7 << _CTRL_REG4__ODR__Pos)
#define _CTRL_REG4__ODR__800HZ   (0x8 << _CTRL_REG4__ODR__Pos)
#define _CTRL_REG4__ODR__1600HZ  (0x9 << _CTRL_REG4__ODR__Pos)

static spi_t _spi = NULL;

static void _Read(uint8_t reg, void* data, uint8_t len) {
    
    if(!_spi)
	return;
    
    reg |= (1 << 7);
    Spi_Transfer(_spi, &reg, NULL, 1, NULL, data, len);
}

static void _Write(uint8_t reg, const void* data, uint8_t len) {
    
    if(!_spi)
	return;
    
    Spi_Transfer(_spi, &reg, NULL, 1, data, NULL, len);
}

static void _Write8(uint8_t reg, uint8_t data) {    
    _Write(reg, &data, 1);
}
/*
static void _Write16(uint8_t reg, uint16_t data) {
    _Write(reg, &data, 2);
}
*/
static uint8_t _Read8(uint8_t reg) {
    uint8_t data;
    _Read(reg, &data, 1);
    return data;
}
/*
static uint16_t _Read16(uint8_t reg) {
    uint16_t data;
    _Read(reg, &data, 1);
    return data;
}
*/
bool Mpu_Create(void) {
    // setup SPI
    spi_options_t spi_opts = {};
    spi_opts.base = SPI1;
    spi_opts.baudrate = 1000000;
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

    // check if created spi bus
    if(!_spi)
	return false;
    
    // check chip ID
    if(_Read8(_WHO_AM_I) != _WHO_AM_I__VAL)
	return false;
    
    // reboot
    _Write8(_CTRL_REG6, _CTRL_REG6__BOOT);
    while(_Read8(_CTRL_REG6 & _CTRL_REG6__BOOT));
    
    // enable at 100Hz
    _Write8(_CTRL_REG4, _CTRL_REG4__ODR__100HZ | _CTRL_REG4__XEN | _CTRL_REG4__YEN | _CTRL_REG4__ZEN);
    
    // enable increment address
    _Write8(_CTRL_REG6, _CTRL_REG6__ADD_INC);
    
    return true;
}

void Mpu_Read(double* x, double* y, double* z) {
    int16_t xyz[3];
    
    // wait for data ready
    while(!(_Read8(_STATUS) & _STATUS__ZYXDA));

    // read data
    _Read(_OUT_X, xyz, 6);
    
    *x = _CONVERT(xyz[0]);
    *y = _CONVERT(xyz[1]);
    *z = _CONVERT(xyz[2]);
}
