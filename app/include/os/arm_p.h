#pragma once

#include <stdint.h>

extern uint32_t SystemCoreClock;
void SystemCoreClockUpdate(void);

#ifndef __enable_irq
#define __enable_irq() __asm volatile ("cpsie i" : : : "memory");
#endif

#ifndef __disable_irq
#define __disable_irq() __asm volatile ("cpsid i" : : : "memory");
#endif

typedef struct  // TODO: move to ARM specific file?
{
    volatile uint32_t CTRL;                   /*!< Offset: 0x000 (R/W)  SysTick Control and Status Register */
    volatile uint32_t LOAD;                   /*!< Offset: 0x004 (R/W)  SysTick Reload Value Register */
    volatile uint32_t VAL;                    /*!< Offset: 0x008 (R/W)  SysTick Current Value Register */
} arm_SysTick_t;

typedef struct // TODO: move to ARM specific file?
{
    uint32_t CPUID;                           /*!< Offset: 0x000 (R/ )  CPUID Base Register */
    volatile uint32_t ICSR;                   /*!< Offset: 0x004 (R/W)  Interrupt Control and State Register */
    volatile uint32_t VTOR;                   /*!< Offset: 0x008 (R/W)  Vector Table Offset Register */
    volatile uint32_t AIRCR;                  /*!< Offset: 0x00C (R/W)  Application Interrupt and Reset Control Register */
    volatile uint32_t SCR;                    /*!< Offset: 0x010 (R/W)  System Control Register */
    volatile uint32_t CCR;                    /*!< Offset: 0x014 (R/W)  Configuration Control Register */
    uint32_t RESERVED1;
    uint32_t SHPR1;
    uint32_t SHPR2;
    uint32_t SHPR3;
    volatile uint32_t SHCSR;                  /*!< Offset: 0x024 (R/W)  System Handler Control and State Register */
} arm_Scb_t;

typedef struct {
    uint8_t reserved[16];      // 0xE000E000
    arm_SysTick_t systick;      // 0xE000E010 - 0xE000E1C
    uint32_t reserved2[824];    // 0xE000E020 - 0xE000ECF
    arm_Scb_t scb;              // 0xE000ED00
} arm_SCS_t;

#define ARM_SCS_BASE (arm_SCS_t *)0xE000E000