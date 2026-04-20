#include "hal/Critical.h"
#include "os/os_p.h"

void os_PutWith(uint32_t key, os_context_t context, os_action_t timeoutAction, uint32_t ticks)
{
    os_entry_t *entry = os_EntryAlloc();
    if (entry == NULL) {
        os_Fail(OS_FAIL_PUT_WITH_ALLOCATION);
        return;
    }

    entry->action = timeoutAction;
    // ticks==0 dispatches timeoutAction immediately via the FIFO; the entry is
    // not on the board, so it's keyless (os_Get won't find it).
    entry->key = (ticks == 0) ? OS_NO_KEY : key;
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
