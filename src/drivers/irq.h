#ifndef _IRQ_H_
#define _IRQ_H_

#include "cmsis.h" // IRQn_Type

typedef IRQn_Type irq_t;

void Irq_Enable(irq_t irq);
void Irq_SetPriority(irq_t irq, uint32_t priority);
void Irq_Set(irq_t irq);
void Irq_Clear(irq_t irq);
uint32_t Irq_GetLowestPriority(void);
uint32_t Irq_GetHighestPriority(void);

#endif // _IRQ_H_
