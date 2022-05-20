#pragma once

typedef enum {
    LED= 0,
    LED_0,
    LED_1,
    LED_2,
    NUMBER_OF_LEDS,
} led_t;

void led_Init(void);
void led_On(led_t led);
void led_Off(led_t led);
void led_Toggle(led_t led);
