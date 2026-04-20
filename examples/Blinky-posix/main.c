/**
 * @file main.c
 * @brief Blinky-posix — EventOS Blinky running on a POSIX host.
 *
 * Same action logic as examples/Blinky: one action toggles LED at 200 ms
 * by re-enqueueing itself through the timer queue. The only difference
 * is the platform — LEDs print to stdout with a timestamp and SysTick
 * is simulated by posix_Tick().
 *
 * LIMITATIONS of this single-threaded demo:
 *   - os_Tick() runs inline between os_Exec() iterations, not in an
 *     ISR. An action that takes longer than 1 ms will delay tick
 *     delivery by that much.
 *   - hal_CriticalBegin/End are no-ops (see posix/source/Critical.c),
 *     so action-vs-ISR races that would show on hardware will not.
 *
 * For a more faithful port run posix_Tick() on its own pthread, driven
 * by a POSIX interval timer, and use a pthread_mutex in Critical.c.
 *
 * Stop the program with Ctrl-C.
 */

#include "Led.h"
#include "os/os.h"
#include "posix.h"

void Blinky(os_context_t context) {
    (void)context;
    led_Toggle(LED);
    os_DoAfter(Blinky, OS_NO_CONTEXT, OS_NO_KEY, 200);
}

int main(void) {
    posix_Init();
    os_Do(Blinky, OS_NO_CONTEXT);
    for (;;) {
        posix_Tick();
        os_Exec();
    }
}
