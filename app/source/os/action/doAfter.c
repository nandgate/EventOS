#include "hal/Critical.h"
#include "os/os_p.h"

os_context_t os_DoAfter(os_action_t action, uint32_t contextSize, uint32_t key, uint32_t ticks)
{
    os_entry_t *entry = os_EntryAlloc();
    if (entry == NULL) {
        os_Fail(OS_FAIL_DO_AFTER_ALLOCATION);
        return NULL;
    }

    entry->action = action;
    entry->key = key;
    entry->ticks = ticks;
    entry->ctx = os_ContextNew(contextSize);

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

    return entry->ctx != NULL ? entry->ctx->data : NULL;
}
