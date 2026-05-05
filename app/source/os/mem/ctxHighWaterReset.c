#include "os/os_p.h"

extern uint32_t os_ctxSmallHighWaterMark;
extern uint32_t os_ctxLargeHighWaterMark;

void os_CtxHighWaterReset(os_ctxBucket_t bucket)
{
    if (bucket == OS_CTX_BUCKET_LARGE) {
        os_ctxLargeHighWaterMark = 0;
    } else {
        os_ctxSmallHighWaterMark = 0;
    }
}
