/**
 * @file main.c
 * @brief Thread — Demonstrates a chained-action thread kicked off by a button.
 *
 * A "thread" is a sequence of actions walking one piece of work through
 * several stages, with each stage passing a shared context forward.
 * This demo spawns one thread per debounced button press and runs it to
 * completion — no loop, no background polling.
 *
 * At boot, LED is silent. The first button press kicks off a thread
 * whose Stage1 starts the 500 ms heartbeat. The heartbeat then runs
 * concurrently with the rest of the thread — on the scope you can see
 * LED toggling on its own 500 ms rhythm while LED_0, LED_1, LED_2 fire
 * at 100 ms spacing. (Press again and another heartbeat starts — the
 * LED blinks faster as more press-threads accumulate heartbeats.)
 *
 * Pipeline (100 ms between stages):
 *
 *   Button press -> Settle -> Stage1 (starts Heartbeat) -> Stage2 -> Stage3
 *
 *   - LED:   silent until first Stage1 runs; then 500 ms forever
 *   - LED_0: toggles when Stage1 runs
 *   - LED_1: toggles when Stage2 runs
 *   - LED_2: toggles when Stage3 runs
 *
 * Each thread gets its own threadState_t context. Tap the button twice
 * within ~200 ms and two threads run concurrently — stages from
 * different threads interleave on the scope, each with its own state.
 *
 * Saleae capture: ../captures/Thread/Thread.sal
 */

#include "bluepill.h"
#include "Button.h"
#include "Led.h"
#include "os/os.h"
#include <stdint.h>

#define DEBOUNCE_KEY   0xB1CE
#define DEBOUNCE_MS    20
#define STAGE_DELAY_MS 100

typedef struct {
    uint32_t threadId;
} threadState_t;

static uint32_t threadCounter;

void Heartbeat(os_context_t context) {
    (void)context;
    led_Toggle(LED);
    os_DoAfter(Heartbeat, OS_NO_CONTEXT, OS_NO_KEY, 500);
}

void Stage3(os_context_t context) {
    (void)context;
    led_Toggle(LED_2);
    // Thread ends here. Context is released automatically when this
    // action returns.
}

void Stage2(os_context_t context) {
    led_Toggle(LED_1);
    os_DoAfterWith(Stage3, context, OS_NO_KEY, STAGE_DELAY_MS);
}

void Stage1(os_context_t context) {
    led_Toggle(LED_0);
    os_Do(Heartbeat, OS_NO_CONTEXT);
    os_DoAfterWith(Stage2, context, OS_NO_KEY, STAGE_DELAY_MS);
}

void Settle(os_context_t context) {
    (void)context;
    threadState_t *state = (threadState_t *)os_Do(Stage1, sizeof(threadState_t));
    state->threadId = ++threadCounter;
}

void ButtonISR(void) {
    os_CancelPending(DEBOUNCE_KEY);
    os_DoAfter(Settle, OS_NO_CONTEXT, DEBOUNCE_KEY, DEBOUNCE_MS);
}

int main(void) {
    bluepill_Init();
    button_Init(ButtonISR);

    while (1) {
        os_Exec();
    }
}
