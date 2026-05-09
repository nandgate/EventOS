#include "os/os_p.h"

os_actionFifo_t    os_actionFifo;
os_ctx_t          *os_ctxLargeFreeList;
uint32_t           os_ctxLargeHighWaterMark;
uint32_t           os_ctxLargeInUseCount;
uint32_t           os_ctxLargePool[OS_CTX_LARGE_COUNT][CTX_SLOT_WORDS(OS_CTX_LARGE_SLOT)];
os_ctx_t          *os_ctxSmallFreeList;
uint32_t           os_ctxSmallHighWaterMark;
uint32_t           os_ctxSmallInUseCount;
uint32_t           os_ctxSmallPool[OS_CTX_SMALL_COUNT][CTX_SLOT_WORDS(OS_CTX_SMALL_SLOT)];
os_entry_t        *os_entryFreeList;
uint32_t           os_entryHighWaterMark;
uint32_t           os_entryInUseCount;
os_entry_t         os_entryPool[OS_MAX_ENTRIES];
os_ctx_t          *os_heldCtx;
os_subscription_t *os_subFreeList;
uint32_t           os_subHighWaterMark;
uint32_t           os_subInUseCount;
os_subscription_t  os_subPool[OS_MAX_SUBSCRIPTIONS];
os_subscription_t *os_subscriptions;
os_entry_t        *os_tQueue;

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