/**
 * @file main.c
 * @brief CancelPending — Demonstrates dynamic timer control.
 *
 * LED blinks via a cancellable timer. Every 3 seconds a controller action
 * cancels the blink and restarts it at a different rate (fast/slow toggle).
 *
 *   - LED:   blinks at the current rate (100ms fast, 500ms slow)
 *   - LED_0: on = fast mode, off = slow mode
 *
 * Saleae capture: ../captures/CancelPending/CancelPending.sal
 */

#include "bluepill.h"
#include "Led.h"
#include "os/os.h"
#include <stdint.h>

#define BLINK_KEY 0xBEEF

#define RATE_FAST 100
#define RATE_SLOW 500

typedef struct {
    uint32_t rate;
} blinkRate_t;

typedef struct {
    uint32_t currentRate;
} controllerState_t;

void Blink(os_context_t context) {
    blinkRate_t *cfg = (blinkRate_t *)context;
    led_Toggle(LED);
    os_DoAfterWith(Blink, context, BLINK_KEY, cfg->rate);
}

void Controller(os_context_t context) {
    controllerState_t *state = (controllerState_t *)context;

    // Cancel the current blink
    os_CancelPending(BLINK_KEY);

    // Toggle the rate
    if (state->currentRate == RATE_FAST) {
        state->currentRate = RATE_SLOW;
        led_Off(LED_0);
    } else {
        state->currentRate = RATE_FAST;
        led_On(LED_0);
    }

    // Restart the blink at the new rate
    blinkRate_t *cfg = (blinkRate_t *)os_Do(Blink, sizeof(blinkRate_t));
    cfg->rate = state->currentRate;

    // Schedule the next rate change, forwarding this context
    os_DoAfterWith(Controller, context, OS_NO_KEY, 3000);
}

int main(void) {
    bluepill_Init();

    // Start blinking fast
    blinkRate_t *cfg = (blinkRate_t *)os_Do(Blink, sizeof(blinkRate_t));
    cfg->rate = RATE_FAST;
    led_On(LED_0);

    // Schedule the first rate change, seeding the controller's starting rate
    controllerState_t *state = (controllerState_t *)os_DoAfter(
        Controller, sizeof(controllerState_t), OS_NO_KEY, 3000);
    state->currentRate = RATE_FAST;

    while (1) {
        os_Exec();
    }
}
