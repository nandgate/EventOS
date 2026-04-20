/**
 * @file main.c
 * @brief Debounce — Software debouncer via cancel + reschedule.
 *
 * A "button" (a wire on PB1 tapped to GND) generates a falling-edge
 * interrupt on EXTI1 for every contact bounce. Rather than dispatching
 * an action for each edge, the ISR:
 *
 *   1. Cancels any pending Settle action (os_CancelPending).
 *   2. Schedules a new Settle action DEBOUNCE_MS in the future.
 *
 * If another bounce arrives within the settle window, the timer
 * restarts. Only when DEBOUNCE_MS elapses with no new edge does Settle
 * actually run, producing one clean "press" signal no matter how noisy
 * the contact is. Compare with the Button example to see the raw
 * bounce pattern this filters.
 *
 *   - LED:   heartbeat, toggles every 500 ms (proof of life)
 *   - LED_0: toggles once per debounced press
 *
 * Saleae capture: ../captures/Debounce/Debounce.sal
 */

#include "bluepill.h"
#include "Button.h"
#include "Led.h"
#include "os/os.h"

#define DEBOUNCE_KEY   0xB1CE
#define DEBOUNCE_MS    20

void Settle(os_context_t context) {
    (void)context;
    led_Toggle(LED_0);
}

void ButtonISR(void) {
    os_CancelPending(DEBOUNCE_KEY);
    os_DoAfter(Settle, OS_NO_CONTEXT, DEBOUNCE_KEY, DEBOUNCE_MS);
}

void Heartbeat(os_context_t context) {
    (void)context;
    led_Toggle(LED);
    os_DoAfter(Heartbeat, OS_NO_CONTEXT, OS_NO_KEY, 500);
}

int main(void) {
    bluepill_Init();
    button_Init(ButtonISR);

    os_Do(Heartbeat, OS_NO_CONTEXT);

    while (1) {
        os_Exec();
    }
}
