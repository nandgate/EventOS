#include "os/os_p.h"
//#include "cmsis_gcc.h"  // must come after os_p.h

os_context_t os_Do(os_action_t action, uint32_t contextSize)
{
    __disable_irq();

    // allocate queue entry and add it to the queue
    os_actionEntry_t *actionEntry = os_MemAlloc(sizeof(os_actionEntry_t));
    while(actionEntry == NULL); // TODO: Error handling for when malloc fails, NULL return- out of memory
    os_FifoAdd(actionEntry);

    // Associate the action with the entry
    actionEntry->action = action;

    // Associate a new context with the action.
    actionEntry->ctx = os_ContextNew(contextSize);

    __enable_irq();
    return actionEntry->ctx->data;
}
