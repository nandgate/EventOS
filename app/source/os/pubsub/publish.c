#include "hal/Critical.h"
#include "os/os_p.h"

extern os_subscription_t *os_subscriptions;

os_context_t os_Publish(uint32_t topic, uint32_t contextSize) {
    os_context_t ctx = NULL;

    hal_CriticalBegin();
    for (os_subscription_t *sub = os_subscriptions; sub != NULL; sub = sub->next) {
        if (sub->topic == topic) {
            if (ctx == NULL) {
                ctx = os_Do(sub->action, contextSize);
            } else {
                os_DoWith(sub->action, ctx);
            }
        }
    }
    if (ctx == NULL) {
        // No subscribers; still return a valid context for the caller to fill in.
        ctx = os_Do(os_NullAction, contextSize);
    }
    hal_CriticalEnd();

    return ctx;
}
