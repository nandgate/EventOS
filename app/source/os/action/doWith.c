#include "hal/Critical.h"
#include "os/os_p.h"

void os_DoWith(os_action_t action, os_context_t context)
{
    os_entry_t *actionEntry = os_EntryAlloc();
    if (actionEntry == NULL) {
        os_Fail(OS_FAIL_DO_WITH_ALLOCATION);
        return;
    }

    actionEntry->action = action;
    actionEntry->key = OS_NO_KEY;
    actionEntry->ctx = os_ContextAcquire(context);

    hal_CriticalBegin();
    os_FifoAdd(actionEntry);
    hal_CriticalEnd();
}
