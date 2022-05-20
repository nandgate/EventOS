#include "os/os_p.h"
//#include "cmsis_gcc.h"  // must come after os_p.h

void os_DoWith(os_action_t action, os_context_t context)
{
    __disable_irq();

    // allocate queue entry and add it to the queue
    os_actionEntry_t *actionEntry = os_MemAlloc(sizeof(os_actionEntry_t));
    while(actionEntry == NULL); // TODO: Error handling for when malloc fails, NULL return- out of memory
    os_FifoAdd(actionEntry);

    // Associate the action with the entry
    actionEntry->action = action;

    // Associate the context with the action.
    actionEntry->ctx = os_ContextReuse(context);

    __enable_irq();
}
