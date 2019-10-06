#include "pin.h"
#include "drivers/clock.h"

// mode register values
#define _MODER_INPUT  0x0ul
#define _MODER_OUTPUT 0x1ul
#define _MODER_ALT    0x2ul
#define _MODER_ANALOG 0x3ul
#define _MODER_MASK   0x3ul

// output type register values
#define _OTYPER_PP    0x0ul
#define _OTYPER_OD    0x1ul
#define _OTYPER_MASK   0x1ul

// output speed register values
#define _OSPEEDR_LOW     0x0ul
#define _OSPEEDR_MED     0x1ul
#define _OSPEEDR_HIGH    0x2ul
#define _OSPEEDR_HIGHEST 0x3ul
#define _OSPEEDR_MASK    0x3ul

// pull up / down register values
#define _PUPDR_NONE 0x0ul
#define _PUPDR_UP   0x1ul
#define _PUPDR_DOWN 0x2ul
#define _PUPDR_MASK 0x3ul

#define _PORT(x) ((GPIO_TypeDef*)x)

bool Pin_Create(const pin_options_t* options) {
    
    // get port
    GPIO_TypeDef* port = _PORT(options->port);
    
    // enable clock
    Clock_EnablePeripheral(port);
    
    // clear mode
    port->MODER &= ~(_MODER_MASK << (options->pin * 2));
    
    switch(options->mode) {
	// input
    case PIN__MODE__INPUT:
	port->MODER |= (_MODER_INPUT << (options->pin * 2));
	break;
	// output
    case PIN__MODE__OUTPUT_LOW:
    case PIN__MODE__OUTPUT_HIGH:
	port->MODER |= (_MODER_OUTPUT << (options->pin * 2));
        
	// set default level
	if(options->mode == PIN__MODE__OUTPUT_LOW)
	    Pin_Clear(options->port, options->pin);
	else
	    Pin_Set(options->port, options->pin);
	
	break;
	
	// analog
    case PIN__MODE__ANALOG:
	port->MODER |= (_MODER_ANALOG << (options->pin * 2));
	break;
	
	// alternate function
    case PIN__MODE__AF0:
    case PIN__MODE__AF1:
    case PIN__MODE__AF2:
    case PIN__MODE__AF3:
    case PIN__MODE__AF4:
    case PIN__MODE__AF5:
    case PIN__MODE__AF6:
    case PIN__MODE__AF7:
    case PIN__MODE__AF8:
    case PIN__MODE__AF9:
    case PIN__MODE__AF10:
    case PIN__MODE__AF11:
    case PIN__MODE__AF12:
    case PIN__MODE__AF13:
    case PIN__MODE__AF14:
    case PIN__MODE__AF15:
	port->MODER |= (_MODER_ALT << (options->pin * 2));
	
	// set alternate function number
	if(options->pin <= 7) {
	    port->AFR[0] &= ~(0xf << (4 * options->pin));
	    port->AFR[0] |= ((options->mode - PIN__MODE__AF0) << (4 * options->pin));
	}
	else {
	    port->AFR[1] &= ~(0xf << (4 * (options->pin - 8)));
	    port->AFR[1] |= ((options->mode - PIN__MODE__AF0) << (4 * (options->pin - 8)));
	}
	
	break;
    default:
	return false;
    }
    
    // attributes
    // open drain (default push-pull)
    port->OTYPER &= ~(_OTYPER_MASK << options->pin);
    if(options->attr & PIN__ATTR__OPEN_DRAIN)
	port->OTYPER |= (_OTYPER_OD << options->pin);
    
    // pull up/down (default none)
    port->PUPDR &= ~(_PUPDR_MASK << (options->pin * 2));
    if(options->attr * PIN__ATTR__PULL_UP)
	port->PUPDR |= (_PUPDR_UP << (options->pin * 2));
    if(options->attr * PIN__ATTR__PULL_DOWN)
	port->PUPDR |= (_PUPDR_DOWN << (options->pin * 2));
    
    // set speed (default low)
    port->OSPEEDR &= ~(_OSPEEDR_MASK << (options->pin * 2));
    if(options->attr & PIN__ATTR__SPEED_MED)
	port->OSPEEDR |= (_OSPEEDR_MED << (options->pin * 2));
    else if(options->attr & PIN__ATTR__SPEED_HIGH)
	port->OSPEEDR |= (_OSPEEDR_HIGH << (options->pin * 2));
    else if(options->attr & PIN__ATTR__SPEED_HIGHEST)
	port->OSPEEDR |= (_OSPEEDR_HIGHEST << (options->pin * 2));

    return true;
}

void Pin_Set(const pin_port_t port, const pin_num_t pin) {
    _PORT(port)->BSRRL |= (1ul << pin);
}

void Pin_Clear(const pin_port_t port, const pin_num_t pin) {
    _PORT(port)->BSRRH |= (1ul << pin);
}

bool Pin_Read(const pin_port_t port, const pin_num_t pin) {
    return _PORT(port)->IDR & (1ul << pin);
}
