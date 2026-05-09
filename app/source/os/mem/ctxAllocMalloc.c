#include "os/os_p.h"
#include <stdlib.h>

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
