#ifndef _PIN_H
#define _PIN_H

#include <stdbool.h>
#include "stm32f4xx.h"

typedef enum {
    PIN__MODE__INPUT,
    PIN__MODE__OUTPUT_LOW,
    PIN__MODE__OUTPUT_HIGH,
    PIN__MODE__ANALOG,
    PIN__MODE__AF0,
    PIN__MODE__AF1,
    PIN__MODE__AF2,
    PIN__MODE__AF3,
    PIN__MODE__AF4,
    PIN__MODE__AF5,
    PIN__MODE__AF6,
    PIN__MODE__AF7,
    PIN__MODE__AF8,
    PIN__MODE__AF9,
    PIN__MODE__AF10,
    PIN__MODE__AF11,
    PIN__MODE__AF12,
    PIN__MODE__AF13,
    PIN__MODE__AF14,
    PIN__MODE__AF15
} pin_mode_t;

#define PIN__ATTR__OPEN_DRAIN     (0x1 << 0)
#define PIN__ATTR__PUSH_PULL      (0x0) // default
#define PIN__ATTR__PULL_UP        (0x1 << 1)
#define PIN__ATTR__PULL_DOWN      (0x1 << 2)
#define PIN__ATTR__PULL_NONE      (0x0) // default
#define PIN__ATTR__SPEED_LOW      (0x0) // default
#define PIN__ATTR__SPEED_MED      (0x1 << 4)
#define PIN__ATTR__SPEED_HIGH     (0x1 << 5)
#define PIN__ATTR__SPEED_HIGHEST  (0x1 << 6)

typedef uint8_t pin_attr_t;
typedef void*   pin_port_t;
typedef uint8_t pin_num_t;

typedef struct {
    pin_port_t port;     // port
    pin_num_t pin;       // number in port
    pin_mode_t mode;     // mode
    pin_attr_t attr;     // attributes
} pin_options_t;

bool Pin_Create(const pin_options_t* options);
void Pin_Set(const pin_port_t port, const pin_num_t pin);
void Pin_Clear(const pin_port_t port, const pin_num_t pin);
bool Pin_Read(const pin_port_t port, const pin_num_t pin);

#endif // _PIN_H
