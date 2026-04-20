/**
 * @file main.c
 * @brief Blinky — The minimal EventOS example and reference port-init sequence.
 *
 * A single action toggles LED and re-enqueues itself every 200 ms. This is
 * the EventOS "heartbeat" pattern — larger examples leave an equivalent
 * blinker running in the background as a visible sign that the scheduler
 * is still pumping. If Blinky stops, the system has locked up.
 *
 * main() also spells the port init out inline (clock, LED driver, critical
 * section, OS, SysTick) rather than hiding it behind a bluepill_Init()
 * helper. Other examples use the helper; this example is the reference
 * for what the helper does.
 *
 *   - LED: toggles every 200 ms (the heartbeat)
 *
 * Saleae capture: (not archived — visual confirmation only).
 */

#include "bluepill.h"
#include "hal/Critical.h"
#include "Led.h"
#include "os/os.h"

void SystemCoreClockUpdate(void);
extern uint32_t SystemCoreClock;

void os_Fail(os_fail_t reason) {
    (void)reason;
    while (1);
}

void Blinky(os_context_t context) {
    (void)context;  // context is not used

    led_Toggle(LED);
    os_DoAfter(Blinky, OS_NO_CONTEXT, OS_NO_KEY, 200);
}

int main(void) {
    SystemCoreClockUpdate();
    led_Init();
    hal_CriticalInit();

    os_Init();
    bluepill_SysTickInit(SystemCoreClock / OS_TICKS_PER_SECOND);

    os_Do(Blinky, OS_NO_CONTEXT);
    while (1) {
        os_Exec();
    }
}

// Newlib stubs:
#include <stddef.h>
#include <sys/reent.h>
int _close_r(struct _reent *ptr, int fd) { return 0; }
int _lseek_r(int fd, int offset, int whence) { return 0; };
int _read_r(struct _reent *r, int fd, void *buf, size_t nbytes) { return 0;}
int _write_r(void *reent, int fd, const char *ptr, int len) { return 0; }
