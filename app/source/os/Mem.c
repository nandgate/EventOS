#include "os/os_p.h"
#include <stdlib.h>

// NOTE: the references to TIM1 and inclusion of the STM32F1xx.h header are for memory management timing testing. They
// are NOT required for the operation of EventOS- and should be removed in a deployed instance.
#include "stm32f1xx.h"

void os_MemInit(void)
{
    TIM1->PSC = 0;  // tick at full speed
    TIM1->CNT = 0;
}

void *os_MemAlloc(uint32_t size)
{
    TIM1->CR1 = TIM_CR1_CEN;    // Enable, up counting

    // Use the system malloc for now (the horror), to be replaced with our own memory management with known properties
    return malloc(size);

    TIM1->CR1 = 0;       // Stop counting
}

void os_MemFree(void *mem)
{
    TIM1->CR1 = TIM_CR1_CEN;    // Enable, up counting

    // Use the system malloc for now (it probably works), to be replaced with owr own memory management
    free(mem);

    TIM1->CR1 = 0;       // Stop counting
}
