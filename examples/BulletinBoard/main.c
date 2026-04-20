/**
 * @file main.c
 * @brief BulletinBoard — Demonstrates the bulletin board.
 *
 * A poster posts a context to the bulletin board every 2 seconds with a
 * 750 ms timeout. A getter tries to retrieve it every 1.5 seconds. Two
 * of the three getter arrivals per cycle land inside the TTL window and
 * hit; the third arrives after the TTL has already expired, so the
 * timeout callback fires and the getter then misses.
 *
 *   - LED:   toggles when the poster posts
 *   - LED_0: toggles when the getter retrieves successfully
 *   - LED_1: toggles on timeout (entry expired before retrieval)
 *
 * Sequence (one 6-second cycle):
 *
 *   Time(s)  Poster             Bulletin board     Getter
 *   -------  -----------------  -----------------  -----------------
 *   0.0      Post (LED toggle)  [data, ttl=0.75s]
 *   0.0                         .                  Get -> HIT (LED_0)
 *                               (empty)
 *   1.5                                            Get -> miss
 *
 *   2.0      Post (LED toggle)  [data, ttl=0.75s]
 *   2.75                        ttl expires
 *                               Timeout -> LED_1
 *   3.0                                            Get -> miss
 *
 *   4.0      Post (LED toggle)  [data, ttl=0.75s]
 *   4.5                         .                  Get -> HIT (LED_0)
 *                               (empty)
 *   6.0      (cycle repeats)
 *
 * Saleae capture: ../captures/BulletinBoard/BulletinBoard.sal
 */

#include "bluepill.h"
#include "Led.h"
#include "os/os.h"
#include <stddef.h>
#include <stdint.h>

#define BB_KEY 0x1234

void Timeout(os_context_t context) {
    (void)context;
    led_Toggle(LED_1);
}

void Producer(os_context_t context) {
    uint32_t *data = (uint32_t *)context;

    led_Toggle(LED);
    *data = 0xCAFE;
    os_PutWith(BB_KEY, data, Timeout, 750);

    os_DoAfter(Producer, sizeof(uint32_t), OS_NO_KEY, 2000);
}

void Consumer(os_context_t context) {
    (void)context;

    os_context_t data = os_Get(BB_KEY);
    if (data != NULL) {
        led_Toggle(LED_0);
    }

    os_DoAfter(Consumer, OS_NO_CONTEXT, OS_NO_KEY, 1500);
}

int main(void) {
    bluepill_Init();

    os_Do(Producer, sizeof(uint32_t));
    os_Do(Consumer, OS_NO_CONTEXT);

    while (1) {
        os_Exec();
    }
}
