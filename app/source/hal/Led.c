#include "hal/Led.h"
#include "stm32f1xx.h"

typedef struct ledIO {
    GPIO_TypeDef *gpioBase;
    __IO uint32_t *configPtr;
    uint32_t setMask;
    uint32_t clearMask;
    uint32_t inputMask;
    uint32_t mode;
    uint32_t config;
} ledIO_t;

static const ledIO_t led_io[NUMBER_OF_LEDS] = {
    [LED] = {
        .gpioBase = GPIOC,
        .configPtr = &(GPIOC->CRH),
        .setMask = GPIO_BSRR_BS13,
        .clearMask = GPIO_BSRR_BR13,
        .inputMask = GPIO_IDR_IDR13,
        .mode= GPIO_CRH_MODE13_Pos,
        .config =GPIO_CRH_CNF13_Msk
    },
    [LED_0] = {
        .gpioBase = GPIOA,
        .configPtr = &(GPIOA->CRL),
        .setMask = GPIO_BSRR_BS0,
        .clearMask = GPIO_BSRR_BR0,
        .inputMask = GPIO_IDR_IDR0,
        .mode= GPIO_CRL_MODE0_Pos,
        .config =GPIO_CRL_CNF0_Msk
    },
    [LED_1] = {
        .gpioBase = GPIOA,
        .configPtr = &(GPIOA->CRL),
        .setMask = GPIO_BSRR_BS1,
        .clearMask = GPIO_BSRR_BR1,
        .inputMask = GPIO_IDR_IDR1,
        .mode= GPIO_CRL_MODE1_Pos,
        .config =GPIO_CRL_CNF1_Msk
    },
    [LED_2] = {
        .gpioBase = GPIOA,
        .configPtr = &(GPIOA->CRL),
        .setMask = GPIO_BSRR_BS2,
        .clearMask = GPIO_BSRR_BR2,
        .inputMask = GPIO_IDR_IDR2,
        .mode= GPIO_CRL_MODE2_Pos,
        .config =GPIO_CRL_CNF2_Msk
    }
};

void led_Init(void) {
    // Enable the clock to the LED port
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN_Msk | RCC_APB2ENR_IOPAEN_Msk;

    // Configure the LED pin to be an output
    for(led_t led= LED; led < NUMBER_OF_LEDS; led++) {
        *led_io[led].configPtr |= (2 << led_io[led].mode);
        *led_io[led].configPtr &= ~(led_io[led].config);
    }
}

void led_On(led_t led) {
    led_io[led].gpioBase->BSRR = led_io[led].clearMask;
}

void led_Off(led_t led) {
    led_io[led].gpioBase->BSRR = led_io[led].setMask;
}

void led_Toggle(led_t led) {
    led_io[led].gpioBase->BSRR = (led_io[led].gpioBase->IDR & led_io[led].inputMask) ?
        led_io[led].clearMask :  led_io[led].setMask;
}
