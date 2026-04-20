#include "bluepill.h"
#include "bluepill_p.h"
#include "hal/Critical.h"
#include "Led.h"
#include "os/os.h"

void bluepill_Init(void) {
    SystemCoreClockUpdate();
    led_Init();
    hal_CriticalInit();
    os_Init();
    bluepill_SysTickInit(SystemCoreClock / OS_TICKS_PER_SECOND);
}

__attribute__((weak))
void os_Fail(os_fail_t reason) {
    (void)reason;
    while (1);
}

__attribute__((weak))
void os_ActionBegin(os_action_t action) {
    (void)action;
}

__attribute__((weak))
void os_ActionEnd(os_action_t action) {
    (void)action;
}
