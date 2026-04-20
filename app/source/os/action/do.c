#include "hal/Critical.h"
#include "os/os_p.h"

os_context_t os_Do(os_action_t action, uint32_t contextSize)
{
    os_entry_t *actionEntry = os_EntryAlloc();
    if (actionEntry == NULL) {
        os_Fail(OS_FAIL_DO_ALLOCATION);
        return NULL;
    }

    actionEntry->action = action;
    actionEntry->key = OS_NO_KEY;
    actionEntry->ctx = os_ContextNew(contextSize);

    hal_CriticalBegin();
    os_FifoAdd(actionEntry);
    hal_CriticalEnd();
    return actionEntry->ctx != NULL ? actionEntry->ctx->data : NULL;
}
