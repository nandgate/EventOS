#include "hal/Critical.h"
#include "os/os_p.h"
#include <stddef.h>

#ifndef OS_CTX_SMALL_SLOT
#define OS_CTX_SMALL_SLOT  16
#endif
#ifndef OS_CTX_SMALL_COUNT
#define OS_CTX_SMALL_COUNT 4
#endif
#ifndef OS_CTX_LARGE_SLOT
#define OS_CTX_LARGE_SLOT  64
#endif
#ifndef OS_CTX_LARGE_COUNT
#define OS_CTX_LARGE_COUNT 4
#endif

#define CTX_SLOT_WORDS(bytes) (((bytes) + sizeof(uint32_t) - 1) / sizeof(uint32_t))

uint32_t   os_ctxSmallPool[OS_CTX_SMALL_COUNT][CTX_SLOT_WORDS(OS_CTX_SMALL_SLOT)];
uint32_t   os_ctxLargePool[OS_CTX_LARGE_COUNT][CTX_SLOT_WORDS(OS_CTX_LARGE_SLOT)];
os_ctx_t  *os_ctxSmallFreeList;
os_ctx_t  *os_ctxLargeFreeList;
uint32_t   os_ctxInUseCount;
uint32_t   os_ctxHighWaterMark;

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
    os_ctxInUseCount = 0;
    os_ctxHighWaterMark = 0;
}

os_ctx_t *os_CtxAlloc(uint32_t size)
{
    os_ctx_t **freeList;

    if (size <= OS_CTX_SMALL_SLOT) {
        freeList = &os_ctxSmallFreeList;
    } else if (size <= OS_CTX_LARGE_SLOT) {
        freeList = &os_ctxLargeFreeList;
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
    os_ctxInUseCount++;
    if (os_ctxInUseCount > os_ctxHighWaterMark) {
        os_ctxHighWaterMark = os_ctxInUseCount;
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
    } else {
        ctx->next = os_ctxLargeFreeList;
        os_ctxLargeFreeList = ctx;
    }
    os_ctxInUseCount--;
    hal_CriticalEnd();
}
