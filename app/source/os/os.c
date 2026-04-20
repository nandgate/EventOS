#include "os/os_p.h"

os_actionFifo_t os_actionFifo;
os_ctx_t *os_heldCtx;
os_entry_t *os_tQueue;
os_subscription_t *os_subscriptions;

void os_Init(void)
{
    os_heldCtx = NULL;
    os_subscriptions = NULL;
    os_actionFifo.os_fifoHead = NULL;
    os_actionFifo.os_fifoTail = NULL;
    os_CtxAllocInit();
    os_EntryAllocInit();
    os_SubAllocInit();
    os_TimerInit();
}

void os_NullAction(os_context_t context) {
    (void)context;
}