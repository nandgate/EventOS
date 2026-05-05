#include "os/os_p.h"

extern uint32_t os_ctxSmallHighWaterMark;
extern uint32_t os_ctxLargeHighWaterMark;

uint32_t os_CtxHighWater(os_ctxBucket_t bucket)
{
    if (bucket == OS_CTX_BUCKET_LARGE) {
        return os_ctxLargeHighWaterMark;
    }
    return os_ctxSmallHighWaterMark;
}
