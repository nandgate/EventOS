#include "os/os_p.h"
#include <stddef.h>

// This code uses the native malloc/free for memory management. The intention is for this to be temporary and
// that custom memory allocation routines be implemented that account for the usage pattern used. It should
// be possible to get O(1) performance with a custom allocator.

os_ctx_t *os_ContextNew(uint32_t size)
{
    // Note: This code over allocates the context struct by at most 4 bytes due to the uint32_t declaration for the
    // data field in the os_contextEntry_t. Technically we need to add (size - sizeof(uint32_t)), but I didn't want to
    // deal with the underflow logic when size < 4. This is "good enough".
    uint32_t entrySize = sizeof(os_ctx_t) + size;
    os_ctx_t *entry = os_MemAlloc(entrySize);
    while(entry == NULL);// TODO: case where MemAlloc failed (is NUll)
    entry->count = 1;
    entry->size = size;
    return entry;
}

void os_ContextUse(os_ctx_t *ctx)
{
    ctx->count--;
    if (ctx->count == 0)
    {
        os_MemFree(ctx);
    }
}

os_ctx_t * os_ContextReuse(os_context_t context)
{
    os_ctx_t *entry = context - offsetof(os_ctx_t, data);
    entry->count++;
    return entry;
}

os_ctx_t *os_GetCtx(os_context_t context) {
    os_ctx_t *entry = context - offsetof(os_ctx_t, data);
    return entry;
}