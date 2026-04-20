/**
 * @file Led.c
 * @brief LED printer for POSIX host demos.
 *
 * Instead of toggling GPIO pins, each LED state change is written to
 * stdout as `[  <ms>] <name> <state>`. Pipe the output through `tee`
 * or redirect to a file to analyze timings the same way you would a
 * logic-analyzer capture.
 */

#include "Led.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>

static struct timespec startTime;
static int ledState[NUMBER_OF_LEDS];

static const char *LedName(led_t led) {
    switch (led) {
        case LED:   return "LED  ";
        case LED_0: return "LED_0";
        case LED_1: return "LED_1";
        case LED_2: return "LED_2";
        default:    return "??   ";
    }
}

static uint32_t ElapsedMs(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint32_t)((now.tv_sec  - startTime.tv_sec)  * 1000
                   +  (now.tv_nsec - startTime.tv_nsec) / 1000000);
}

void led_Init(void) {
    clock_gettime(CLOCK_MONOTONIC, &startTime);
    for (led_t i = LED; i < NUMBER_OF_LEDS; i++) {
        ledState[i] = 0;
    }
    setvbuf(stdout, NULL, _IOLBF, 0);
}

void led_On(led_t led) {
    ledState[led] = 1;
    printf("[%6u ms] %s ON\n", ElapsedMs(), LedName(led));
}

void led_Off(led_t led) {
    ledState[led] = 0;
    printf("[%6u ms] %s off\n", ElapsedMs(), LedName(led));
}

void led_Toggle(led_t led) {
    if (ledState[led]) { led_Off(led); }
    else               { led_On(led);  }
}
