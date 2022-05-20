#include "os/os_p.h"
//#include "cmsis_gcc.h"  // must come after os_p.h

os_context_t os_DoAfter(os_action_t action, uint32_t contextSize, uint32_t ticks)
{
    // When tick is zero no need to mess with timer mechanics, just do it now...
    if (ticks == 0) {
        return os_Do(action, contextSize);
    }

    __disable_irq();

    // allocate timer queue entry and add it to the queue
    os_tqEntry_t *tqEntry = os_MemAlloc(sizeof(os_tqEntry_t));
    while(tqEntry == NULL); // TODO: Error handling for when malloc fails, NULL return- out of memory

    tqEntry->ticks = ticks;
    tqEntry->ctx = os_ContextNew(contextSize);
    tqEntry->action = action;
    os_TimerAdd(tqEntry);

    __enable_irq();
    return tqEntry->ctx->data;
}
