#include "stm32f1xx.h"

void led_Init(void) {
    // Enable the clock to the LED port
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN_Pos;

    // Setup the pin to be an output
    GPIOC->CRH |= (2 << GPIO_CRH_MODE13_Pos);
}

void led_On(void) {
    GPIOC->BSRR = GPIO_BSRR_BR13;
}

void led_Off(void) {
    GPIOC->BSRR = GPIO_BSRR_BS13;
}

void led_Toggle(void) {
    GPIOC->BSRR = (GPIOC->IDR & GPIO_IDR_IDR13) ? GPIO_BSRR_BR13 : GPIO_BSRR_BS13;
}
