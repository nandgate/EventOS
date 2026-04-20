#include "os/os_p.h"

os_ctx_t *os_ContextNew(uint32_t size)
{
    if (size == 0) {
        return NULL;
    }
    uint32_t entrySize = sizeof(os_ctx_t) + size;
    os_ctx_t *entry = os_CtxAlloc(entrySize);
    if (entry == NULL) {
        os_Fail(OS_FAIL_CONTEXT_ALLOCATION);
        return NULL;
    }
    entry->count = 1;
    entry->size = size;
    return entry;
}
