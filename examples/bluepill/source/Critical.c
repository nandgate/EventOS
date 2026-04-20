/**
 * @file Critical.c
 * @brief Critical section implementation for ARM Cortex-M.
 *
 * Provides nestable critical sections by tracking the nesting depth
 * and only disabling/enabling interrupts at the outermost level.
 */

#include "arm_p.h"
#include "hal/Critical.h"
#include <stdint.h>

volatile uint32_t hal_criticalNesting;

void hal_CriticalInit(void) {
    hal_criticalNesting = 0;
}

void hal_CriticalBegin(void) {
    __disable_irq();
    hal_criticalNesting++;
}

void hal_CriticalEnd(void) {
    if (0 < hal_criticalNesting) {
        hal_criticalNesting--;
        if (0 == hal_criticalNesting) {
            __enable_irq();
        }
    }
}
