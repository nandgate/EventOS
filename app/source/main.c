#include "hal/Led.h"
#include "hal/Timer.h"
#include "os/os.h"
#include <stdbool.h>
#include <stdlib.h>

#include "stm32f1xx.h"

void SystemCoreClockUpdate(void);
extern uint32_t SystemCoreCLock;
uint64_t memTicks= 0;
uint32_t ticks = 0;

#define MAX_TIME        5000
#define MAX_CTX_SIZE    4096
#define TIME_WINDOW     2

void ITM_SendStr(char *str) {
    while(*str) {
        ITM_SendChar(*str++);
    }
}

const char *hexChar = "0123456789ABCDEF";
void ITM_SendHex(uint32_t value) {
    ITM_SendChar(hexChar[(value >> 28) & 0xF]);
    ITM_SendChar(hexChar[(value >> 24) & 0xF]);
    ITM_SendChar(hexChar[(value >> 20) & 0xF]);
    ITM_SendChar(hexChar[(value >> 16) & 0xF]);
    ITM_SendChar(hexChar[(value >> 12) & 0xF]);
    ITM_SendChar(hexChar[(value >> 8) & 0xF]);
    ITM_SendChar(hexChar[(value >> 4) & 0xF]);
    ITM_SendChar(hexChar[(value >> 0) & 0xF]);
}

uint32_t getRandomTime(void) {
    return rand() % MAX_TIME;
}

void t0_StartCycle(os_context_t context);
void t0_phase0(os_context_t context);
void t0_phase1(os_context_t context);
uint32_t t0_time;
uint32_t t0_ctxSize;
bool t0_error;

void t0_StartCycle(os_context_t context) {
    (void) context;
    t0_time = getRandomTime();
    t0_ctxSize = rand() % MAX_CTX_SIZE;

    ITM_SendStr("T0: ");
    ITM_SendHex(t0_time);
    ITM_SendChar('\n');

    os_DoAfter(t0_phase0, t0_ctxSize, t0_time);
    led_On(LED_0);
    if (timer_Start(TIMER_0) != TIMER_OK) {
        t0_error = true;
    }
}

void t0_phase0(os_context_t context) {
    (void) context;
    uint16_t ticks;

    if(timer_Stop(TIMER_0) == TIMER_OK) {
        if (timer_ReadTicks(TIMER_0, &ticks) == TIMER_OK) {
            ITM_SendStr("T0P0: ");
            ITM_SendHex(ticks);
            ITM_SendChar('\n');

            if ((ticks+TIME_WINDOW) >= t0_time) {
                os_DoAfterWith(t0_phase1, context, t0_time);
                timer_Start(TIMER_0);
                led_Off(LED_0);
                return;
            }
        }
    }
    t0_error= true;
}

void t0_phase1(os_context_t context) {
    (void) context;
    uint16_t ticks;

    if(timer_Stop(TIMER_0) == TIMER_OK) {
        if (timer_ReadTicks(TIMER_0, &ticks) == TIMER_OK) {
            ITM_SendStr("T0P1: ");
            ITM_SendHex(ticks);
            ITM_SendChar('\n');

            if ((ticks+TIME_WINDOW) >= t0_time) {
                os_Do(t0_StartCycle, OS_NO_CONTEXT);
                return;
            }
        }
    }
    t0_error= true;
}

void t1_StartCycle(os_context_t context);
void t1_phase0(os_context_t context);
void t1_phase1(os_context_t context);
uint32_t t1_time;
uint32_t t1_ctxSize;
bool t1_error;

void t1_StartCycle(os_context_t context) {
    (void) context;
    t1_time = getRandomTime();
    t1_ctxSize = rand() % MAX_CTX_SIZE;

    ITM_SendStr("T1: ");
    ITM_SendHex(t1_time);
    ITM_SendChar('\n');

    os_DoAfter(t1_phase0, t1_ctxSize, t1_time);
    led_On(LED_1);
    if (timer_Start(TIMER_1) != TIMER_OK) {
        t1_error = true;
    }
}

void t1_phase0(os_context_t context) {
    (void) context;
    uint16_t ticks;

    if(timer_Stop(TIMER_1) == TIMER_OK) {
        if (timer_ReadTicks(TIMER_1, &ticks) == TIMER_OK) {
            ITM_SendStr("T1P0: ");
            ITM_SendHex(ticks);
            ITM_SendChar('\n');

            if ((ticks+TIME_WINDOW) >= t1_time) {
                os_DoAfterWith(t1_phase1, context, t1_time);
                timer_Start(TIMER_1);
                led_Off(LED_1);
                return;
            }
        }
    }
    t1_error= true;
}

void t1_phase1(os_context_t context) {
    (void) context;
    uint16_t ticks;

    if(timer_Stop(TIMER_1) == TIMER_OK) {
        if (timer_ReadTicks(TIMER_1, &ticks) == TIMER_OK) {
            ITM_SendStr("T1P1: ");
            ITM_SendHex(ticks);
            ITM_SendChar('\n');

            if ((ticks+TIME_WINDOW) >= t1_time) {
                os_Do(t1_StartCycle, OS_NO_CONTEXT);
                return;
            }
        }
    }
    t1_error= true;
}

void t2_StartCycle(os_context_t context);
void t2_phase0(os_context_t context);
void t2_phase1(os_context_t context);
uint32_t t2_time;
uint32_t t2_ctxSize;
bool t2_error;

void t2_StartCycle(os_context_t context) {
    (void) context;
    t2_time = getRandomTime();
    t2_ctxSize = rand() % MAX_CTX_SIZE;

    ITM_SendStr("T2: ");
    ITM_SendHex(t2_time);
    ITM_SendChar('\n');

    os_DoAfter(t2_phase0, t2_ctxSize, t2_time);
    led_On(LED_2);
    if (timer_Start(TIMER_2) != TIMER_OK) {
        t2_error = true;
    }
}

void t2_phase0(os_context_t context) {
    (void) context;
    uint16_t ticks;

    if(timer_Stop(TIMER_2) == TIMER_OK) {
        if (timer_ReadTicks(TIMER_2, &ticks) == TIMER_OK) {
            ITM_SendStr("T2P0: ");
            ITM_SendHex(ticks);
            ITM_SendChar('\n');

            if ((ticks+TIME_WINDOW) >= t2_time) {
                os_DoAfterWith(t2_phase1, context, t2_time);
                timer_Start(TIMER_2);
                led_Off(LED_2);
                return;
            }
        }
    }
    t2_error= true;
}

void t2_phase1(os_context_t context) {
    (void) context;
    uint16_t ticks;

    if(timer_Stop(TIMER_2) == TIMER_OK) {
        if (timer_ReadTicks(TIMER_2, &ticks) == TIMER_OK) {
            ITM_SendStr("T2P1: ");
            ITM_SendHex(ticks);
            ITM_SendChar('\n');

            if ((ticks+TIME_WINDOW) >= t2_time) {
                os_Do(t2_StartCycle, OS_NO_CONTEXT);
                return;
            }
        }
    }
    t2_error= true;
}

void Blinky(os_context_t context) {
    (void)context;
    led_Toggle(LED);
    os_DoAfterWith(Blinky, context, 250);

    // Memory management timing report
    memTicks += TIM1->CNT;
    TIM1->CNT = 0;

    ITM_SendStr("memticks: ");
    ITM_SendHex((memTicks >> 32UL));
    ITM_SendHex(memTicks & 0xFFFFFFFFUL);
    ITM_SendChar('\n');

    ITM_SendStr("ticks: ");
    ITM_SendHex(++ticks);
    ITM_SendChar('\n');
}

int main(void) {
    SystemCoreClockUpdate();
    timer_Init();   // Used for testing the OS
    led_Init();     // Uses for testing the OS

    os_Init(SystemCoreCLock / OS_CLOCKS_PER_TICK);
    t0_error= false;

    os_Do(Blinky, OS_NO_CONTEXT);
    os_Do(t0_StartCycle, OS_NO_CONTEXT);
    os_Do(t1_StartCycle, OS_NO_CONTEXT);
    os_Do(t2_StartCycle, OS_NO_CONTEXT);
    while (1) {
        os_Exec();
    }
}
