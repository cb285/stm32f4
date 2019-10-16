#include "irq.h"
#include <math.h>

#define _LOWEST_PRIO (pow(2, __NVIC_PRIO_BITS) - 1)
#define _HIGHEST_PRIO 0

void Irq_Enable(irq_t irq) {
    NVIC_EnableIRQ(irq);
}

void Irq_SetPriority(irq_t irq, uint32_t priority) {
    NVIC_SetPriority(irq, priority);
}

void Irq_Set(irq_t irq) {
    NVIC_SetPendingIRQ(irq);
}

void Irq_Clear(irq_t irq) {
    NVIC_ClearPendingIRQ(irq);
}

uint32_t Irq_GetLowestPriority(void) {
    return _LOWEST_PRIO;
}

uint32_t Irq_GetHighestPriority(void) {
    return _HIGHEST_PRIO;
}
