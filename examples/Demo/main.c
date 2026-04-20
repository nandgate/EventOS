/**
 * @file main.c
 * @brief Demo — Combined example exercising all EventOS features.
 *
 * Each LED demonstrates a different feature simultaneously:
 *   - LED (PC13):  Heartbeat blink via os_DoAfter (always-running indicator)
 *   - LED_0 (PA0): Driven by pub/sub — subscribes to a topic published by a timer
 *   - LED_1 (PA1): Controlled via bulletin board — posted/retrieved between actions
 *   - LED_2 (PA2): Cancellable blink — cycles between fast/slow via os_CancelPending
 *
 * Saleae capture: ../captures/Demo/Demo.sal
 */

#include "bluepill.h"
#include "Led.h"
#include "os/os.h"
#include <stddef.h>
#include <stdint.h>

#define TOPIC_TICK      1
#define BB_KEY          0x1234
#define BLINK_KEY       0xBEEF

#define RATE_FAST       100
#define RATE_SLOW       500

// --- LED: Heartbeat ---

void Heartbeat(os_context_t context) {
    (void)context;
    led_Toggle(LED);
    os_DoAfter(Heartbeat, OS_NO_CONTEXT, OS_NO_KEY, 500);
}

// --- LED_0: Pub/Sub ---

void TickPublisher(os_context_t context) {
    (void)context;
    os_Publish(TOPIC_TICK, OS_NO_CONTEXT);
    os_DoAfter(TickPublisher, OS_NO_CONTEXT, OS_NO_KEY, 750);
}

void TickSubscriber(os_context_t context) {
    (void)context;
    led_Toggle(LED_0);
}

// --- LED_1: Bulletin board ---

void BbTimeout(os_context_t context) {
    (void)context;
    led_Off(LED_1);
}

void BbPoster(os_context_t context) {
    (void)context;
    uint32_t *data = os_Put(BB_KEY, sizeof(uint32_t), BbTimeout, 1000);
    *data = 0xCAFE;
    os_DoAfter(BbPoster, OS_NO_CONTEXT, OS_NO_KEY, 2000);
}

void BbGetter(os_context_t context) {
    (void)context;
    os_context_t data = os_Get(BB_KEY);
    if (data != NULL) {
        led_On(LED_1);
    }
    os_DoAfter(BbGetter, OS_NO_CONTEXT, OS_NO_KEY, 1500);
}

// --- LED_2: CancelPending ---

typedef struct {
    uint32_t rate;
} blinkRate_t;

void BlinkLed2(os_context_t context) {
    blinkRate_t *cfg = (blinkRate_t *)context;
    led_Toggle(LED_2);
    os_DoAfterWith(BlinkLed2, context, BLINK_KEY, cfg->rate);
}

void RateController(os_context_t context) {
    (void)context;
    static uint32_t currentRate = RATE_FAST;

    os_CancelPending(BLINK_KEY);

    currentRate = (currentRate == RATE_FAST) ? RATE_SLOW : RATE_FAST;

    blinkRate_t *cfg = (blinkRate_t *)os_Do(BlinkLed2, sizeof(blinkRate_t));
    cfg->rate = currentRate;

    os_DoAfter(RateController, OS_NO_CONTEXT, OS_NO_KEY, 3000);
}

// --- Main ---

int main(void) {
    bluepill_Init();

    // Heartbeat (LED)
    os_Do(Heartbeat, OS_NO_CONTEXT);

    // Pub/Sub (LED_0)
    os_Subscribe(TOPIC_TICK, TickSubscriber);
    os_Do(TickPublisher, OS_NO_CONTEXT);

    // Bulletin board (LED_1)
    os_Do(BbPoster, OS_NO_CONTEXT);
    os_Do(BbGetter, OS_NO_CONTEXT);

    // CancelPending (LED_2)
    blinkRate_t *cfg = (blinkRate_t *)os_Do(BlinkLed2, sizeof(blinkRate_t));
    cfg->rate = RATE_FAST;
    os_DoAfter(RateController, OS_NO_CONTEXT, OS_NO_KEY, 3000);

    while (1) {
        os_Exec();
    }
}
