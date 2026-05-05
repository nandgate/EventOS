#include "os/os_p.h"
#include <stdlib.h>

uint32_t os_ctxSmallInUseCount;
uint32_t os_ctxLargeInUseCount;
uint32_t os_ctxSmallHighWaterMark;
uint32_t os_ctxLargeHighWaterMark;

void os_CtxAllocInit(void)
{
}

os_ctx_t *os_CtxAlloc(uint32_t size)
{
    return (os_ctx_t *)malloc(size);
}

void os_CtxFree(os_ctx_t *ctx)
{
    free(ctx);
}
