#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx.h"
#include "board_config.h"
#include "drivers/board.h"
#include "drivers/clock.h"
#include "drivers/pin.h"
#include "utils/debug.h"
#include "utils/delay.h"
#include "drivers/spi.h"

static void _SpiTransfer(const spi_t handle, const uint8_t* tx1, uint8_t* rx1, uint32_t len1, const uint8_t* tx2, uint8_t* rx2, uint32_t len2) {
    uint32_t i;
    for(i = 0; i < len1; i++) {
	Spi_Transmit(handle, tx1[i]);
	rx1[i] = Spi_Receive(handle);
	i++;
    }
    
    for(i = 0; i < len2; i++) {
	Spi_Transmit(handle, tx2[i]);
	rx2[i] = Spi_Receive(handle);
	i++;
    }
}

static uint8_t _ReadReg(const spi_t handle, uint8_t addr) {
    uint8_t tx1 = (addr << 1) | 1;

    uint8_t dump;
    uint8_t rx1;
    uint8_t rx2;
    
    _SpiTransfer(handle, &tx1, &dump, 1, &rx1, &rx2, 1);

    Debug_Log(DEBUG__LEVEL__INFO, "rx1 = 0x%x", rx1);
    Debug_Log(DEBUG__LEVEL__INFO, "rx2 = 0x%x", rx2);
    
    return rx2;
}

int main(void) {
    /*
    // enable PWR interface clock
    RCC->APB1ENR |= 1 << 28;

    // enable voltage scale 2 mode
    PWR->CR &= ~PWR_CR_VOS;

    // wait for voltage scale to be set
    while(!(PWR->CSR & (1 << 14)));
    */

    // init clock
    //Board_Init();
    
    pin_options_t green;
    green.port = GPIOD;
    green.pin = LED_GREEN;
    green.mode = PIN__MODE__OUTPUT_LOW;
    
    pin_options_t red;
    red.port = GPIOD;
    red.pin = LED_RED;
    red.mode = PIN__MODE__OUTPUT_LOW;
    
    Pin_Create(&green);
    Pin_Create(&red);

    Debug_Create();

    spi_options_t spi_opts = {};
    spi_opts.base = SPI1;
    spi_opts.baudrate = 4000;
    spi_opts.data_len = SPI__DATA_LEN__8BITS;
    spi_opts.polarity = SPI__POLARITY__IDLE_HIGH;
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
    spi_opts.cs_mode = PIN__MODE__AF5;

    spi_t spi = Spi_Create(&spi_opts);
    
    while(1) {
	Debug_Log(DEBUG__LEVEL__INFO, "WHO_AM_I = 0x%x", _ReadReg(spi, 0xf));
	
	Pin_Set(GPIOD, LED_GREEN);
	Delay_Ms(50);
	Pin_Clear(GPIOD, LED_GREEN);

	Delay_Ms(50);
	
	Pin_Set(GPIOD, LED_RED);
	Delay_Ms(50);
	Pin_Clear(GPIOD, LED_RED);
	
	Delay_Ms(50);
    }
	
    return 0;
}
