/**
 * @file main.c
 * @brief MultiBlink — Demonstrates contexts carrying state, at a fast rate.
 *
 * Four LEDs blink at different rates, all driven by a single action function.
 * Each LED gets its own context containing which LED to toggle and how fast.
 * os_DoAfterWith reuses the context on each cycle, avoiding re-allocation.
 *
 * Rates are chosen pairwise-coprime (3, 5, 7, 11 ms) so every phase
 * combination between the four chains is exercised within the LCM
 * (1155 ms). Toggles are too fast to see visually — the scope is the
 * intended audience.
 *
 * Saleae capture: ../captures/MultiBlink/MultiBlink.sal
 */

#include "bluepill.h"
#include "Led.h"
#include "os/os.h"
#include <stdint.h>

typedef struct {
    led_t led;
    uint32_t rate;
} blink_t;

void Blink(os_context_t context) {
    blink_t *blink = (blink_t *)context;
    led_Toggle(blink->led);
    os_DoAfterWith(Blink, context, OS_NO_KEY, blink->rate);
}

int main(void) {
    bluepill_Init();

    blink_t *ctx;

    // Pairwise-coprime rates — every phase combination is exercised over
    // the LCM (3*5*7*11 = 1155 ms).
    ctx = (blink_t *)os_Do(Blink, sizeof(blink_t));
    ctx->led = LED;
    ctx->rate = 3;

    ctx = (blink_t *)os_Do(Blink, sizeof(blink_t));
    ctx->led = LED_0;
    ctx->rate = 5;

    ctx = (blink_t *)os_Do(Blink, sizeof(blink_t));
    ctx->led = LED_1;
    ctx->rate = 7;

    ctx = (blink_t *)os_Do(Blink, sizeof(blink_t));
    ctx->led = LED_2;
    ctx->rate = 11;

    while (1) {
        os_Exec();
    }
}
