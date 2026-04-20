#include "hal/Critical.h"
#include "os/os_p.h"

void os_DoAfterWith(os_action_t action, os_context_t context, uint32_t key, uint32_t ticks)
{
    os_entry_t *entry = os_EntryAlloc();
    if (entry == NULL) {
        os_Fail(OS_FAIL_DO_AFTER_WITH_ALLOCATION);
        return;
    }

    entry->action = action;
    entry->key = key;
    entry->ticks = ticks;
    entry->ctx = os_ContextAcquire(context);

    hal_CriticalBegin();
    if (key != OS_NO_KEY) {
        // Set semantics: replace any existing pending entry with the same key.
        os_CancelPending(key);
    }
    if (ticks == 0) {
        os_FifoAdd(entry);
    } else {
        os_TimerAdd(entry);
    }
    hal_CriticalEnd();
}
