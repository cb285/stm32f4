#include "pin.h"
#include "drivers/clock.h"
#include "drivers/irq.h"

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

#define _MAX_IRQS 16
#define _PRIO_DISABLED 0xfffffffful

#define _MIN(x, y) ((x < y)? x : y)

#define _PORT(x) ((GPIO_TypeDef*)x)

typedef struct {
    bool used;
    bool enabled;
    pin_port_t port;
    pin_num_t pin;
    uint32_t irq_priority;         // interrupt priority
    pin_irq_handler_t irq_handler; // interrupt handler function
} _irq_slot_t;

static uint32_t _exti04_priorities[5] = { _PRIO_DISABLED, _PRIO_DISABLED, _PRIO_DISABLED, _PRIO_DISABLED, _PRIO_DISABLED };
static uint32_t _exti59_priority = _PRIO_DISABLED;
static uint32_t _exti1015_priority = _PRIO_DISABLED;

static bool _exti_used[16] = {};
static _irq_slot_t _irq_slots[_MAX_IRQS] = {};

void _HandleIrq(uint8_t exti) {
    // find and call handler
    for(uint8_t i = 0; i < _MAX_IRQS; i++) {
	if(_irq_slots[i].used && _irq_slots[i].enabled && _irq_slots[i].pin == exti)
	    _irq_slots[i].irq_handler();
    }
    
    // clear pending bit
    EXTI->PR = (1ul << exti);
}

void EXTI0_IRQHandler(void) {
    _HandleIrq(0);
}

void EXTI1_IRQHandler(void) {
    _HandleIrq(1);
}

void EXTI2_IRQHandler(void) {
    _HandleIrq(2);
}

void EXTI3_IRQHandler(void) {
    _HandleIrq(3);
}

void EXTI4_IRQHandler(void) {
    _HandleIrq(4);
}

void EXTI9_5_IRQHandler(void) {
    for(uint8_t bit = 5; bit < 10; bit++)
	if(EXTI->PR & (1ul << bit))
	    _HandleIrq(bit);
}

void EXTI15_10_IRQHandler(void) {
    for(uint8_t bit = 10; bit < 16; bit++)
	if(EXTI->PR & (1ul << bit))
	    _HandleIrq(bit);
}

// find a currently allocated slot
static _irq_slot_t* _FindIrqSlot(const pin_port_t port, const pin_num_t pin) {
    for(uint8_t i = 0; i < _MAX_IRQS; i++) {
	if(_irq_slots[i].used) {
	    if((_irq_slots[i].port == port) && (_irq_slots[i].pin == pin))
		return &_irq_slots[i];
	}
    }
    return NULL;
}

// find available irq slot
static _irq_slot_t* _AllocateIrqSlot(const pin_port_t port, const pin_num_t pin) {

    // check if EXTI is already being used
    if(_exti_used[pin])
	return NULL;

    // prevent re-registering same pin
    if(_FindIrqSlot(port, pin))
	return NULL;
    
    // find empty slot
    for(uint8_t i = 0; i < _MAX_IRQS; i++) {
	if(!_irq_slots[i].used) {
	    _irq_slots[i].used = true;
	    _exti_used[pin] = true;
	    return &_irq_slots[i];
	}
    }
    return NULL;
}
/*
static void _FreeIrqSlot(const pin_port_t port, const pin_num_t pin) {

    // check if actually allocated
    if(!_exti_used[pin])
	return;
    
    // find and free slot
    for(uint8_t i = 0; i < _MAX_IRQS; i++) {
	if((_irq_slots[i].port == port) && (_irq_slots[i].pin == pin)) {
	    _irq_slots[i].used = false;
	    _exti_used[pin] = false;
	    return;
	}
    }
}
*/
// get highest priority (lowest number) of currently registered interrupts
static uint32_t _GetIrqPriority(pin_num_t pin, uint32_t priority) {
    if(pin <= 4)
	return _MIN(priority, _exti04_priorities[pin]);
    else if(pin <= 9)
	return _MIN(priority, _exti59_priority);
    else
	return _MIN(priority, _exti1015_priority);
}

static bool _RegisterIrq(const pin_options_t* options) {
    
    // check if should setup interrupt
    if(options->irq_mode == PIN__IRQ_MODE__NONE)
	return true;
    
    // check if irq handler is given
    if(!options->irq_handler)
	return false;
    
    // allocate irq slot
    _irq_slot_t* slot = _AllocateIrqSlot(options->port, options->pin);
    
    // check if got slot
    if(!slot)
	return false;
    
    // fill slot info
    slot->port = options->port;
    slot->pin = options->pin;
    slot->irq_priority = options->irq_priority;
    slot->irq_handler = options->irq_handler;
    
    // enable SYSCFG peripheral clock
    Clock_EnablePeripheral(SYSCFG);
    
    // select correct EXTICR (0 is EXTICR1)
    uint8_t exticr = options->pin / 4;
    
    // get position of EXTIx field inside EXTICR
    uint8_t exticr_pos = (options->pin % 4) * 4;
    
    // check if already being used?
    //if(SYSCFG->EXTICR[exticr] & (0xf << exticr_pos))
    //return false;
    
    // set EXTIx field in EXTICR (port A = 0, port B = 1, ...)
    uint8_t exticr_extix = ((uint32_t)options->port - (uint32_t)GPIOA) / ((uint32_t)GPIOB - (uint32_t)GPIOA);
    
    // set EXTICR
    SYSCFG->EXTICR[exticr] |= ((uint32_t)exticr_extix << exticr_pos);
    
    // unmask interrupt
    EXTI->IMR |= (1ul << options->pin);
    
    // enable rising/falling interrupt
    if(options->irq_mode == PIN__IRQ_MODE__RISING)
	EXTI->RTSR |= (1ul << options->pin);
    else
	EXTI->FTSR |= (1ul << options->pin);
    
    // select IRQn
    uint32_t irq;
    if(options->pin <= 4)
	irq = EXTI0_IRQn + options->pin;
    else if(options->pin <= 9)
	irq = EXTI9_5_IRQn;
    else
	irq = EXTI15_10_IRQn;
    
    // set IRQn priority
    Irq_SetPriority(irq, _GetIrqPriority(options->pin, options->irq_priority));
    
    // enable
    slot->enabled = true;
    Irq_Enable(irq);
    
    return true;
}

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
    if(options->attr & PIN__ATTR__PULL_UP)
	port->PUPDR |= (_PUPDR_UP << (options->pin * 2));
    if(options->attr & PIN__ATTR__PULL_DOWN)
	port->PUPDR |= (_PUPDR_DOWN << (options->pin * 2));
    
    // set speed (default low)
    port->OSPEEDR &= ~(_OSPEEDR_MASK << (options->pin * 2));
    if(options->attr & PIN__ATTR__SPEED_MED)
	port->OSPEEDR |= (_OSPEEDR_MED << (options->pin * 2));
    else if(options->attr & PIN__ATTR__SPEED_HIGH)
	port->OSPEEDR |= (_OSPEEDR_HIGH << (options->pin * 2));
    else if(options->attr & PIN__ATTR__SPEED_HIGHEST)
	port->OSPEEDR |= (_OSPEEDR_HIGHEST << (options->pin * 2));
    
    // setup interrupt (if enabled)
    return _RegisterIrq(options);
}

void Pin_EnableIrq(const pin_port_t port, const pin_num_t pin) {
    
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
