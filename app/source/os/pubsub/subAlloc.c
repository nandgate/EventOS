#include "hal/Critical.h"
#include "os/os_p.h"

extern os_subscription_t *os_subFreeList;
extern uint32_t           os_subInUseCount;
extern uint32_t           os_subHighWaterMark;

os_subscription_t *os_SubAlloc(void)
{
    hal_CriticalBegin();
    os_subscription_t *slot = os_subFreeList;
    if (slot != NULL) {
        os_subFreeList = slot->next;
        os_subInUseCount++;
        if (os_subInUseCount > os_subHighWaterMark) {
            os_subHighWaterMark = os_subInUseCount;
        }
    }
    hal_CriticalEnd();
    return slot;
}
