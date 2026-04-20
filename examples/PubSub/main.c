/**
 * @file main.c
 * @brief PubSub — Demonstrates publish/subscribe with multiple topics.
 *
 * Two topics publish at different rates:
 *   - TOPIC_HEARTBEAT: every 500 ms
 *   - TOPIC_SECOND:    every 2 s
 *
 * LEDs demonstrate both fan-out (many subscribers on one topic) and
 * fan-in (one subscriber on many topics):
 *   - LED:   toggles on every TOPIC_HEARTBEAT
 *   - LED_0: toggles every other TOPIC_HEARTBEAT (uses a counter)
 *   - LED_1: toggles on every TOPIC_SECOND
 *   - LED_2: one subscriber registered on BOTH topics, toggles
 *            whenever either fires
 *
 * Publishers and subscribers are fully decoupled. Neither publisher
 * knows who is listening, and subscribers on one topic are unaware of
 * the other topic.
 *
 * Saleae capture: ../captures/PubSub/PubSub.sal
 */

#include "bluepill.h"
#include "Led.h"
#include "os/os.h"
#include <stdint.h>

#define TOPIC_HEARTBEAT 1
#define TOPIC_SECOND    2

void Heartbeat(os_context_t context) {
    (void)context;
    os_Publish(TOPIC_HEARTBEAT, OS_NO_CONTEXT);
    os_DoAfter(Heartbeat, OS_NO_CONTEXT, OS_NO_KEY, 500);
}

void SlowPublisher(os_context_t context) {
    (void)context;
    os_Publish(TOPIC_SECOND, OS_NO_CONTEXT);
    os_DoAfter(SlowPublisher, OS_NO_CONTEXT, OS_NO_KEY, 2000);
}

void ToggleLed(os_context_t context) {
    (void)context;
    led_Toggle(LED);
}

void ToggleLedHalf(os_context_t context) {
    (void)context;
    static uint32_t count = 0;
    count++;
    if (count % 2 == 0) {
        led_Toggle(LED_0);
    }
}

void ToggleLed1(os_context_t context) {
    (void)context;
    led_Toggle(LED_1);
}

void ToggleLed2(os_context_t context) {
    (void)context;
    led_Toggle(LED_2);
}

int main(void) {
    bluepill_Init();

    os_Subscribe(TOPIC_HEARTBEAT, ToggleLed);
    os_Subscribe(TOPIC_HEARTBEAT, ToggleLedHalf);
    os_Subscribe(TOPIC_HEARTBEAT, ToggleLed2);

    os_Subscribe(TOPIC_SECOND, ToggleLed1);
    os_Subscribe(TOPIC_SECOND, ToggleLed2);

    os_Do(Heartbeat, OS_NO_CONTEXT);
    os_Do(SlowPublisher, OS_NO_CONTEXT);
    while (1) {
        os_Exec();
    }
}
