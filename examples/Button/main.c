/**
 * @file main.c
 * @brief Button — Demonstrates ISR-to-action handoff.
 *
 * A "button" (a wire on PB1 tapped to GND) generates an EXTI1
 * falling-edge interrupt via the internal pull-up. The ISR does the
 * minimum possible work: it calls os_Do() to post an action into the
 * FIFO and returns. The real work (toggling LED_0) runs later in
 * cooperative context, outside the ISR.
 *
 *   - LED:   heartbeat, toggles every 500 ms (proof of life)
 *   - LED_0: toggles once per dispatched OnButtonPress action
 *
 * No debouncing is performed, so each physical tap typically produces
 * several LED_0 toggles as the contact bounces.
 *
 * Saleae capture: ../captures/Button/Button.sal
 */

#include "bluepill.h"
#include "Button.h"
#include "Led.h"
#include "os/os.h"

void OnButtonPress(os_context_t context) {
    (void)context;
    led_Toggle(LED_0);
}

void ButtonISR(void) {
    os_Do(OnButtonPress, OS_NO_CONTEXT);
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
