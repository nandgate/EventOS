#include "hal/Critical.h"
#include "os/os_p.h"

extern os_subscription_t *os_subFreeList;
extern uint32_t           os_subInUseCount;

void os_SubFree(os_subscription_t *sub)
{
    if (sub == NULL) {
        os_Fail(OS_FAIL_SUB_FREE);
        return;
    }
    hal_CriticalBegin();
    sub->next = os_subFreeList;
    os_subFreeList = sub;
    os_subInUseCount--;
    hal_CriticalEnd();
}
