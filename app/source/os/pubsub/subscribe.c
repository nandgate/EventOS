#include "hal/Critical.h"
#include "os/os_p.h"

extern os_subscription_t *os_subscriptions;

void os_Subscribe(uint32_t topic, os_action_t action) {
    os_subscription_t *sub;

    hal_CriticalBegin();
    // Ignore duplicate (topic, action) subscriptions.
    for (sub = os_subscriptions; sub != NULL; sub = sub->next) {
        if (sub->topic == topic && sub->action == action) {
            hal_CriticalEnd();
            return;
        }
    }

    sub = os_SubAlloc();
    if (sub == NULL) {
        hal_CriticalEnd();
        os_Fail(OS_FAIL_SUBSCRIBE_ALLOCATION);
        return;
    }

    sub->topic = topic;
    sub->action = action;
    sub->next = os_subscriptions;
    os_subscriptions = sub;
    hal_CriticalEnd();
}
