#include "hal/Critical.h"
#include "Led.h"
#include "os/os.h"
#include "posix.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static struct timespec startTime;
static uint32_t lastTickMs;

static uint32_t NowMs(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint32_t)((now.tv_sec  - startTime.tv_sec)  * 1000
                   +  (now.tv_nsec - startTime.tv_nsec) / 1000000);
}

void posix_Init(void) {
    clock_gettime(CLOCK_MONOTONIC, &startTime);
    lastTickMs = 0;
    led_Init();
    hal_CriticalInit();
    os_Init();
}

void posix_Tick(void) {
    uint32_t now = NowMs();
    if (lastTickMs == now) {
        // No tick boundary crossed since we were last called. Yield briefly
        // so the host CPU doesn't spin at 100%. 100 us is well below the
        // 1 ms tick resolution, so it does not meaningfully affect timing.
        struct timespec nap = { .tv_sec = 0, .tv_nsec = 100 * 1000 };
        nanosleep(&nap, NULL);
        return;
    }
    while (lastTickMs < now) {
        os_Tick();
        lastTickMs++;
    }
}

__attribute__((weak))
void os_Fail(os_fail_t reason) {
    fprintf(stderr, "os_Fail(%d) — aborting\n", (int)reason);
    exit(1);
}

__attribute__((weak))
void os_ActionBegin(os_action_t action) {
    (void)action;
}

__attribute__((weak))
void os_ActionEnd(os_action_t action) {
    (void)action;
}
