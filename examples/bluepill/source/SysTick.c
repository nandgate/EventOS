/**
 * @file SysTick.c
 * @brief SysTick timer setup and ISR for ARM Cortex-M.
 *
 * Configures the ARM SysTick timer to generate periodic interrupts and
 * routes them to os_Tick(). The tick rate is determined by the reload
 * value passed to bluepill_SysTickInit().
 */

#include "arm_p.h"
#include "bluepill_p.h"
#include "os/os.h"

void bluepill_SysTickInit(uint32_t sysTicksPerOsTick) {
    arm_SCS_t *system = ARM_SCS_BASE;

    system->systick.LOAD = sysTicksPerOsTick;
    system->scb.SHPR3 |= ARM_SHPR3_SYSTICK_PRI_LOWEST;
    system->systick.VAL = 0;
    system->systick.CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler(void) {
    os_Tick();
}
