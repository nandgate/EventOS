#include "os/os_p.h"

os_subscription_t  os_subPool[OS_MAX_SUBSCRIPTIONS];
os_subscription_t *os_subFreeList;
uint32_t           os_subInUseCount;
uint32_t           os_subHighWaterMark;

void os_SubAllocInit(void)
{
    os_subFreeList = &os_subPool[0];
    for (uint32_t i = 0; i < OS_MAX_SUBSCRIPTIONS - 1; i++) {
        os_subPool[i].next = &os_subPool[i + 1];
    }
    os_subPool[OS_MAX_SUBSCRIPTIONS - 1].next = NULL;
    os_subInUseCount = 0;
    os_subHighWaterMark = 0;
}
