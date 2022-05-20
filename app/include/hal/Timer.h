#pragma once
#include "stdint.h"

typedef enum {
    TIMER_0,
    TIMER_1,
    TIMER_2,
    NUMBER_OF_TIMERS
} timer_t;

typedef enum {
    TIMER_OK = 0,
    TIMER_ALREADY_STARTED,
    TIMER_RUNNING,
} timer_Status_t;

void timer_Init(void);
timer_Status_t timer_Start(timer_t timer);
timer_Status_t timer_Stop(timer_t timer);
timer_Status_t timer_ReadTicks(timer_t timer, uint16_t *value);
