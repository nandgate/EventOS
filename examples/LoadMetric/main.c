/**
 * @file main.c
 * @brief LoadMetric — visualize FIFO depth under ISR load.
 *
 * Drive the EXTI1 input (PB1) with a function generator. Each falling
 * edge triggers the ISR, which posts an OnEdge action to the FIFO.
 * FM-modulate the carrier across the OS's service rate and the FIFO
 * depth rides up and down — a live demonstration of EventOS.md §2.6
 * (queue depth as the load metric).
 *
 * After every action dispatch, os_ActionEnd reads os_EntryInUse() and
 * emits the current depth as a 3-bit binary value on LED_0 / LED_1 /
 * LED_2. Depth is saturated at 6 (`110`) so the value 7 (`111`,
 * all-on) is reserved to signal os_Fail — e.g. arrival rate exceeded
 * service rate for long enough to exhaust OS_MAX_ENTRIES.
 *
 * Channel mapping on the scope:
 *   LED_0 — FIFO depth, bit 0 (value 1)
 *   LED_1 — FIFO depth, bit 1 (value 2)
 *   LED_2 — FIFO depth, bit 2 (value 4)
 *   value 7 (all three LEDs ON) — os_Fail tripped; system halted.
 *
 * Probe PB1 on a spare scope channel to see the arrival-edge pattern
 * alongside the depth response.
 *
 * Saleae captures: ../captures/LoadMetric-baseline/ and
 * ../captures/LoadMetric-ride-80/. See results.md in this directory
 * for the test setup, service-rate measurement, and cycle analysis.
 */

#include "bluepill.h"
#include "Button.h"
#include "Led.h"
#include "os/os.h"

#define DEPTH_SATURATE 6u
#define DEPTH_FAIL     7u

static void emitDepth(uint32_t value) {
    if (value & 1u) led_On(LED_0);  else led_Off(LED_0);
    if (value & 2u) led_On(LED_1);  else led_Off(LED_1);
    if (value & 4u) led_On(LED_2);  else led_Off(LED_2);
}

void os_ActionEnd(os_action_t action) {
    (void)action;
    // os_EntryInUse includes the entry currently executing (freed only after
    // this hook returns). Subtract 1 so the display shows pending depth —
    // i.e. 0 means "FIFO empty, nothing waiting behind me."
    uint32_t d = os_EntryInUse();
    d = (d > 0u) ? d - 1u : 0u;
    if (d > DEPTH_SATURATE) {
        d = DEPTH_SATURATE;
    }
    emitDepth(d);
}

void os_Fail(os_fail_t reason) {
    (void)reason;
    emitDepth(DEPTH_FAIL);
    while (1);
}

void OnEdge(os_context_t context) {
    (void)context;
    // Intentionally trivial — depth should reflect arrival rate, not
    // action-body cost.
}

void ButtonISR(void) {
    os_Do(OnEdge, OS_NO_CONTEXT);
}

int main(void) {
    bluepill_Init();
    button_Init(ButtonISR);

    while (1) {
        os_Exec();
    }
}
