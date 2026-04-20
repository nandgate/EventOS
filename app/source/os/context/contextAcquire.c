#include "os/os_p.h"
#include <stddef.h>

// No critical section: ISRs only create fresh contexts (os_ContextNew),
// never Acquire an existing one, so no ISR can race on this refcount.
os_ctx_t *os_ContextAcquire(os_context_t context)
{
    if (context == NULL) {
        return NULL;
    }
    os_ctx_t *entry = context - offsetof(os_ctx_t, data);
    entry->count++;
    return entry;
}
