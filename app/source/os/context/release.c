#include "os/os_p.h"

// No critical section: ISRs never hold a reference to an existing context,
// so the refcount is only touched serially by cooperative actions. os_CtxFree
// takes its own critical section around the pool mutation.
void os_ContextRelease(os_ctx_t *ctx)
{
    if (ctx == NULL) {
        return;
    }
    ctx->count--;
    if (ctx->count == 0) {
        os_CtxFree(ctx);
    }
}
