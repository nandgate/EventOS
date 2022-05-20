#include "stm32F1xx.h"

uint32_t SystemCoreCLock;

void SystemCoreClockUpdate(void) {
    SystemCoreCLock = 64000000;
}

static void InitFlash(void)
{
    // Two wait states: SYSCLK >= 48MHz
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY_Msk) | FLASH_ACR_LATENCY_1;
}

static void InitClocks(void)
{
    // PLL:  HSI/2 * 16 = 8 MHz/2 * 16 = 64 MHz (max HSI rate)
    // APB1: SYSCLK/2 = 32MHz   (must be < 32MHz)
    // APB2: SYSCLK/2 = 32MHz   (match APB1)
    // ADC:  APB2/8 = 4MHz      (must be < 14 MHz)
    RCC->CFGR = RCC_CFGR_PLLMULL16 | RCC_CFGR_ADCPRE_DIV8 | RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_PPRE2_DIV2 | RCC_CFGR_SW_HSI;

    // Enable PLL and wait for it to lock
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));    // busy wait

    // Switch to PLL
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | RCC_CFGR_SW_PLL;
}

static void InitTimers(void)
{
    // Initialize Timer 1 for use
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;      // Enable clock
    RCC->APB2RSTR = RCC_APB2RSTR_TIM1RST;   // Set the reset bit
    RCC->APB2RSTR = 0;                      // Clear the reset bit

    // Initialize Timers 2, 3 & 4 for use
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM4EN;        // Enable clock
    RCC->APB1RSTR = RCC_APB1RSTR_TIM2RST | RCC_APB1RSTR_TIM3RST | RCC_APB1RSTR_TIM4RST; // Set the reset bit
    RCC->APB1RSTR = 0;                      // Clear the reset bit
}

static void InitPorts(void)
{
    // Initialize ports A & C for use

    // Enable clock to ports
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;

    // Reset the ports
    RCC->APB2RSTR = RCC_APB2RSTR_IOPARST | RCC_APB2RSTR_IOPBRST | RCC_APB2RSTR_IOPCRST;
    RCC->APB2RSTR = 0;
}

// This function is called by the reset startup code BEFORE main and the C runtime
// is fully initialized. Best practice is to restrict behavior to system HW setup only.
void SystemInit(void)
{
    // NOTE: The order here is important!
    InitFlash();
    InitClocks();
    InitTimers();
    InitPorts();
}
