#include "hal/Critical.h"
#include "os/os_p.h"
#include <stddef.h>

extern os_ctx_t *os_ctxLargeFreeList;
extern uint32_t  os_ctxLargeHighWaterMark;
extern uint32_t  os_ctxLargeInUseCount;
extern uint32_t  os_ctxLargePool[OS_CTX_LARGE_COUNT][CTX_SLOT_WORDS(OS_CTX_LARGE_SLOT)];
extern os_ctx_t *os_ctxSmallFreeList;
extern uint32_t  os_ctxSmallHighWaterMark;
extern uint32_t  os_ctxSmallInUseCount;
extern uint32_t  os_ctxSmallPool[OS_CTX_SMALL_COUNT][CTX_SLOT_WORDS(OS_CTX_SMALL_SLOT)];

static void buildFreeList(uint32_t *pool, uint32_t slotWords, uint32_t count, os_ctx_t **head)
{
    *head = NULL;
    if (count == 0) {
        return;
    }
    *head = (os_ctx_t *)pool;
    os_ctx_t *prev = *head;
    for (uint32_t i = 1; i < count; i++) {
        prev->next = (os_ctx_t *)&pool[i * slotWords];
        prev = prev->next;
    }
    prev->next = NULL;
}

void os_CtxAllocInit(void)
{
    buildFreeList((uint32_t *)os_ctxSmallPool,
                  CTX_SLOT_WORDS(OS_CTX_SMALL_SLOT),
                  OS_CTX_SMALL_COUNT,
                  &os_ctxSmallFreeList);
    buildFreeList((uint32_t *)os_ctxLargePool,
                  CTX_SLOT_WORDS(OS_CTX_LARGE_SLOT),
                  OS_CTX_LARGE_COUNT,
                  &os_ctxLargeFreeList);
    os_ctxSmallInUseCount = 0;
    os_ctxLargeInUseCount = 0;
    os_ctxSmallHighWaterMark = 0;
    os_ctxLargeHighWaterMark = 0;
}

os_ctx_t *os_CtxAlloc(uint32_t size)
{
    os_ctx_t **freeList;
    uint32_t  *inUseCount;
    uint32_t  *highWaterMark;

    if (size <= OS_CTX_SMALL_SLOT) {
        freeList = &os_ctxSmallFreeList;
        inUseCount = &os_ctxSmallInUseCount;
        highWaterMark = &os_ctxSmallHighWaterMark;
    } else if (size <= OS_CTX_LARGE_SLOT) {
        freeList = &os_ctxLargeFreeList;
        inUseCount = &os_ctxLargeInUseCount;
        highWaterMark = &os_ctxLargeHighWaterMark;
    } else {
        return NULL;
    }

    hal_CriticalBegin();
    os_ctx_t *slot = *freeList;
    if (slot == NULL) {
        hal_CriticalEnd();
        return NULL;
    }

    *freeList = slot->next;
    (*inUseCount)++;
    if (*inUseCount > *highWaterMark) {
        *highWaterMark = *inUseCount;
    }
    hal_CriticalEnd();
    return slot;
}

void os_CtxFree(os_ctx_t *ctx)
{
    hal_CriticalBegin();
    if ((OS_CTX_SMALL_COUNT > 0) &&
        (void *)ctx >= (void *)os_ctxSmallPool &&
        (void *)ctx <  (void *)&os_ctxSmallPool[OS_CTX_SMALL_COUNT]) {
        ctx->next = os_ctxSmallFreeList;
        os_ctxSmallFreeList = ctx;
        os_ctxSmallInUseCount--;
    } else {
        ctx->next = os_ctxLargeFreeList;
        os_ctxLargeFreeList = ctx;
        os_ctxLargeInUseCount--;
    }
    hal_CriticalEnd();
}
