/**
 * @file Button.c
 * @brief Button input on PB1 with EXTI1 falling-edge interrupt.
 *
 * Configures PB1 as an input with the internal pull-up enabled. Tapping
 * the pin to GND generates a falling-edge interrupt on EXTI1. The
 * user-provided callback is invoked from ISR context after the pending
 * bit is cleared.
 */

#include "Button.h"
#include "stm32f1xx.h"
#include <stddef.h>

static button_isrCallback_t button_callback;

void button_Init(button_isrCallback_t callback) {
    button_callback = callback;

    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;

    // PB1: input (MODE=00) with pull-up/down enabled (CNF=10)
    GPIOB->CRL = (GPIOB->CRL & ~(GPIO_CRL_MODE1_Msk | GPIO_CRL_CNF1_Msk))
               | (0x2u << GPIO_CRL_CNF1_Pos);

    // Select pull-up (write 1 to ODR bit)
    GPIOB->ODR |= GPIO_ODR_ODR1;

    // Route EXTI1 to port B
    AFIO->EXTICR[0] = (AFIO->EXTICR[0] & ~AFIO_EXTICR1_EXTI1_Msk)
                    | AFIO_EXTICR1_EXTI1_PB;

    EXTI->FTSR |= EXTI_FTSR_TR1;   // falling edge
    EXTI->PR = EXTI_PR_PR1;        // clear any latched pending
    EXTI->IMR |= EXTI_IMR_MR1;     // unmask

    NVIC_EnableIRQ(EXTI1_IRQn);
}

void EXTI1_Handler(void) {
    if (EXTI->PR & EXTI_PR_PR1) {
        EXTI->PR = EXTI_PR_PR1;
        if (button_callback != NULL) {
            button_callback();
        }
    }
}
