#include "hal/Timer.h"
#include "stm32f1xx.h"

extern uint32_t SystemCoreCLock;

static TIM_TypeDef* timer_timerBase[NUMBER_OF_TIMERS] = {
    [TIMER_0] = TIM2,
    [TIMER_1] = TIM3,
    [TIMER_2] = TIM4
};

void timer_Init(void) {
    for(timer_t timer= TIMER_0; timer < NUMBER_OF_TIMERS; timer++) {
        TIM_TypeDef* tim = timer_timerBase[timer];
        tim->PSC = (SystemCoreCLock / 1) / 1000; // tick at 1KHz
    }
}

timer_Status_t timer_Start(timer_t timer) {
    TIM_TypeDef* tim = timer_timerBase[timer];
    if (!(tim->CR1 & TIM_CR1_CEN)) {
        // Timer not enabled
        tim->CNT = 0;
        tim->CR1 = TIM_CR1_CEN;    // Enable, up counting
        return TIMER_OK;
    }
    return TIMER_ALREADY_STARTED;
}

timer_Status_t timer_Stop(timer_t timer) {
    TIM_TypeDef* tim = timer_timerBase[timer];
    tim->CR1 = 0;       // Stop counting
    return TIMER_OK;
}

timer_Status_t timer_ReadTicks(timer_t timer, uint16_t *value) {
    TIM_TypeDef* tim = timer_timerBase[timer];
    if (!(tim->CR1 & TIM_CR1_CEN)) {
        // Timer not enabled
        *value = tim->CNT;
        return TIMER_OK;
    }
    return TIMER_RUNNING;
}