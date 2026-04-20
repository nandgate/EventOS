#include "hal/Critical.h"
#include "os/os_p.h"

extern os_subscription_t *os_subscriptions;

void os_PublishWith(uint32_t topic, os_context_t context) {
    hal_CriticalBegin();
    for (os_subscription_t *sub = os_subscriptions; sub != NULL; sub = sub->next) {
        if (sub->topic == topic) {
            os_DoWith(sub->action, context);
        }
    }
    hal_CriticalEnd();
}
