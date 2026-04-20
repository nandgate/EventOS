#include "hal/Critical.h"
#include "os/os_p.h"

void os_CancelPending(uint32_t key) {
    hal_CriticalBegin();
    os_TimerRemove(key);
    os_FifoCancel(key);
    hal_CriticalEnd();
}
