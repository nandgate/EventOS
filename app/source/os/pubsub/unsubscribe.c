#include "hal/Critical.h"
#include "os/os_p.h"

extern os_subscription_t *os_subscriptions;

void os_Unsubscribe(uint32_t topic, os_action_t action) {
    hal_CriticalBegin();
    os_subscription_t **cursor = &os_subscriptions;
    while (*cursor != NULL) {
        if ((*cursor)->topic == topic && (*cursor)->action == action) {
            os_subscription_t *found = *cursor;
            *cursor = found->next;
            hal_CriticalEnd();
            os_SubFree(found);
            return;
        }
        cursor = &(*cursor)->next;
    }
    hal_CriticalEnd();
}
