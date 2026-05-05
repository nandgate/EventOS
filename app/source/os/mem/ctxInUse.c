#include "os/os_p.h"

extern uint32_t os_ctxSmallInUseCount;
extern uint32_t os_ctxLargeInUseCount;

uint32_t os_CtxInUse(os_ctxBucket_t bucket)
{
    if (bucket == OS_CTX_BUCKET_LARGE) {
        return os_ctxLargeInUseCount;
    }
    return os_ctxSmallInUseCount;
}
