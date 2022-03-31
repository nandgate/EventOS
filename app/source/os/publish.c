#include "os/os_p.h"
#include <stdlib.h>

os_subscription_t *os_subscriptions;

os_context_t os_Publish(uint32_t topic, uint32_t contextSize) {
    // Find matching topic
    os_subscription_t *subscription=  os_subscriptions;
    while(subscription != NULL) {
        if (subscription->topic == topic) {
            int i= 0;
            // There is always at least one action for the topic.
            os_context_t ctx= os_Do(subscription->actions[i++], contextSize);

            // Reuse that context with the other actions.
            while ((i < OS_NUMBER_OF_SUBS) &&  (subscription->actions[i] != NULL)) {
                os_DoWith(subscription->actions[i++], ctx);
            }
            return ctx;
        }
        subscription = subscription->next;
    }

    // Topic not found, return a context and do nothing with it.
    return os_Do(os_NullAction, contextSize);
}

