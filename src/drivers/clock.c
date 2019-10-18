#include "clock.h"
#include "utils/debug.h"
#include "stm32f4xx.h"

// HSI
static _clock_t _hsi = {
    .freq = 16000000, // 16MHz
    .enabled = true
};

// HSE
static _clock_t _hse = {
    .freq = 8000000, // 8MHz
    .enabled = false
};

// PLL
static _clock_t _pll = {
    .freq = 0, // unset until PLL is configured
    .enabled = false
};

// system clock source is hsi on reset
static _clock_t* _system_clock = &_hsi;

bool Clock_Enable(mclock_t clock) {
    switch(clock) {
    case CLOCK__HSI:
	// turn on
	RCC->CR |= RCC_CR_HSION;
		
	// wait until ready
	while(!(RCC->CR & RCC_CR_HSIRDY));

	// update enabled status
	_hsi.enabled = true;
		
	break;
		
    case CLOCK__HSE:
	// turn on
	RCC->CR |= RCC_CR_HSEON;
		
	// wait until ready
	while(!(RCC->CR & RCC_CR_HSERDY));

	// update enabled status
	_hse.enabled = true;
		
	break;
		
    case CLOCK__PLL:
	// turn on
	RCC->CR |= RCC_CR_PLLON;
		
	// wait until ready
	while(!(RCC->CR & RCC_CR_PLLRDY));
		
	// update enabled status
	_pll.enabled = true;
		
	break;

	// invalid clock
    default:
	Debug_Log(DEBUG__LEVEL__ERROR, "invalid clock");
		
	return false;
    }
	
    // success
    return true;
}

bool Clock_Disable(mclock_t clock) {
    switch(clock) {
    case CLOCK__HSI:
	// turn off
	RCC->CR &= ~RCC_CR_HSION;
	
	// update enabled status
	_hsi.enabled = false;
		
	break;
		
    case CLOCK__HSE:
	// turn off
	RCC->CR &= ~RCC_CR_HSEON;

	// update enabled status
	_hse.enabled = false;
	
	break;
	
    case CLOCK__PLL:
	// turn off
	RCC->CR &= ~RCC_CR_PLLON;
	
	// update enabled status
	_pll.enabled = false;
		
	break;

	// invalid clock
    default:
	Debug_Log(DEBUG__LEVEL__ERROR, "invalid clock");
		
	return false;
    }
    
    // success
    return true;
}

bool Clock_SetSystemSource(mclock_t clock)
{
    // enable clock before setting as source
    if(!Clock_Enable(clock))
	return false;
    
    switch(clock) {
    case CLOCK__HSI:
	// switch to source
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_HSI;
	
	// update current system clock source
	_system_clock = &_hsi;
	
	break;
		
    case CLOCK__HSE:
	// switch to source
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_HSE;
	
	// update current system clock source
	_system_clock = &_hse;
	
	break;
		
    case CLOCK__PLL:

	// @TODO: setup flash for higher clock speed
	FLASH->ACR |= FLASH_ACR_LATENCY_5WS; // five wait states is the minimum for 168MHz clock
	FLASH->ACR |= FLASH_ACR_PRFTEN;      // enable prefetch
	
	// switch to source
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	
	// update current system clock source
	_system_clock = &_pll;
	
	break;
	
	// invalid clock
    default:
	Debug_Log(DEBUG__LEVEL__ERROR, "invalid clock");
	
	return false;
    }
    
    // success
    return true;
}

uint32_t Clock_GetFreq(mclock_t clock) {
    switch(clock) {
    case CLOCK__HSI:
	return _hsi.freq;
    case CLOCK__HSE:
	return _hse.freq;
    case CLOCK__PLL:
	return _pll.freq;
    case CLOCK__SYSCLK:
	return _system_clock->freq;
    default:
	return 0;
    }
}

bool Clock_SetPllSource(mclock_t clock) {
    // check if PLL is enabled
    if(_pll.enabled) {
	Debug_Log(DEBUG__LEVEL__ERROR, "pll is enabled");
	return false;
    }
    
    switch(clock) {
    case(CLOCK__HSI):
	// set source
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLSRC;
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSI;
		
	// update source
	_pll.source = &_hsi;
		
	break;

    case(CLOCK__HSE):
	// set source
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLSRC;
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE;

	// update source
	_pll.source = &_hse;
		
	break;

	// invalid clock
    default:
	Debug_Log(DEBUG__LEVEL__ERROR, "invalid source clock");
	
	return false;
    }
	
    // success
    return true;
}

// VCO = source * (N / M)
// PLL = VCO / P
// PLLCLK = (N * source) / (M * P)
// N: 50-432
// M: 0-63
// P: 0-3, 0 = 2, 1 = 4, 2 = 6, 3 = 8
bool Clock_ConfigPll(mclock_t source_clock, uint32_t pllm, uint32_t plln, uint32_t pllp) {
    uint32_t vco_freq;
    uint32_t vco_input_freq;
    bool pllq_found = false;
    uint32_t pll_source_freq;
    uint32_t pll_freq;
    uint8_t pllq;
    
    // check if pll is enabled
    if(_pll.enabled == true) {
	Debug_Log(DEBUG__LEVEL__ERROR, "pll is enabled");
	return false;
    }
	
    // set pll source clock
    if(!Clock_SetPllSource(source_clock)) {
	Debug_Log(DEBUG__LEVEL__ERROR, "pll source couldn't be enabled");
	return false;
    }
	
    // get pll source clock frequency
    pll_source_freq = ((_clock_t*)_pll.source)->freq;
	
    // validate inputs
    // check plln
    if(!(50 <= plln && plln <= 432)) {
	Debug_Log(DEBUG__LEVEL__ERROR, "plln is out of range");
	return false;
    }
	
    // check pllp
    if(!(((pllp % 2) == 0) && (2 <= pllp && pllp <= 8))) {
	Debug_Log(DEBUG__LEVEL__ERROR, "pllp is out of range");
	return false;
    }

    // check pllm
    if(!(2 <= pllm && pllm <= 63)) {
	Debug_Log(DEBUG__LEVEL__ERROR, "pllm is out of range");
	return false;
    }

    // get potential vco input frequency
    vco_input_freq = pll_source_freq / pllm;

    // get potential vco output frequency
    vco_freq = pll_source_freq * (plln / pllm);

    // check vco input frequency is in range
    if(!(1000000 <= vco_input_freq && vco_input_freq <= 2000000)) {
	Debug_Log(DEBUG__LEVEL__ERROR, "vco input frequency is out of range");
	return false;
    }
	
    // find pllq setting that creates 48MHz
    for(pllq = 2; pllq <= 15; pllq++)
	if((vco_freq / pllq) == 48000000) {
	    pllq_found = true;
	    break;
	}
	
    // check if could find a valid pllq setting
    if(!pllq_found) {
	Debug_Log(DEBUG__LEVEL__ERROR, "couldn't create 48MHz pllq clock");
	return false;
    }
	
    // get potential pll output frequency
    pll_freq = vco_freq / pllp;
	
    // check if vco output frequency is in range
    if(!(100000000 <= vco_freq && vco_freq <= 432000000)) {
	Debug_Log(DEBUG__LEVEL__ERROR, "vco frequency is out of range");
	return false;
    }

    // check if pll output frequency is in range
    if(!(pll_freq <= 168000000)) {
	Debug_Log(DEBUG__LEVEL__ERROR, "pll output freq is out of range");
	return false;
    }

    // set plln
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN;
    RCC->PLLCFGR |= plln << 6;
    
    // set pllp
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP;
    RCC->PLLCFGR |= (((pllp / 2) - 1) << 16);
    
    // set pllm
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM;
    RCC->PLLCFGR |= pllm;
    
    // set pllq
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLQ;
    RCC->PLLCFGR |= (pllq << 24);
    
    // update pll frequency
    _pll.freq = pll_freq;
	
    // success
    return true;
}

void Clock_EnablePeripheral(const void* base) {
    // GPIO
    if(base == GPIOA)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    else if(base == GPIOB)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    else if(base == GPIOC)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    else if(base == GPIOD)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    else if(base == GPIOE)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    else if(base == GPIOF)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
    else if(base == GPIOG)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
    else if(base == GPIOH)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;
    else if(base == GPIOI)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN;
    // USART
    else if(base == USART1)
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    else if(base == USART2)
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    else if(base == USART3)
	RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    else if(base == UART4)
	RCC->APB1ENR |= RCC_APB1ENR_UART4EN;
    else if(base == UART5)
	RCC->APB1ENR |= RCC_APB1ENR_UART5EN;
    else if(base == USART6)
	RCC->APB2ENR |= RCC_APB2ENR_USART6EN;

    // SPI
    else if(base == SPI1)
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    else if(base == SPI2)
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    else if(base == SPI3)
	RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;

    // TIM
    else if(base == TIM1)
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    else if(base == TIM2)
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    else if(base == TIM3)
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    else if(base == TIM4)
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    else if(base == TIM5)
	RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
    else if(base == TIM6)
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    else if(base == TIM7)
	RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    else if(base == TIM8)
	RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;
    else if(base == TIM9)
	RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;
    else if(base == TIM10)
	RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
    else if(base == TIM11)
	RCC->APB2ENR |= RCC_APB2ENR_TIM11EN;
    else if(base == TIM12)
	RCC->APB1ENR |= RCC_APB1ENR_TIM12EN;
    else if(base == TIM13)
	RCC->APB1ENR |= RCC_APB1ENR_TIM13EN;
    else if(base == TIM14)
	RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;

    // SYSCFG
    if(base == SYSCFG)
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
}

// @TODO
uint32_t Clock_GetPeripheralFreq(const void* base) {
    return Clock_GetFreq(CLOCK__SYSCLK);
}
