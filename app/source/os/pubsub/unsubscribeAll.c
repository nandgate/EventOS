#include "hal/Critical.h"
#include "os/os_p.h"

extern os_subscription_t *os_subscriptions;

void os_UnsubscribeAll(uint32_t topic) {
    hal_CriticalBegin();
    os_subscription_t **cursor = &os_subscriptions;
    while (*cursor != NULL) {
        if ((*cursor)->topic == topic) {
            os_subscription_t *found = *cursor;
            *cursor = found->next;
            os_SubFree(found);
        } else {
            cursor = &(*cursor)->next;
        }
    }
    hal_CriticalEnd();
}
