#include "os/os_p.h"
#include <stdlib.h>

os_subscription_t *os_subscriptions;

void os_PublishWith(uint32_t topic, os_context_t context) {
    // Find matching topic
    os_subscription_t *subscription=  os_subscriptions;
    while(subscription != NULL) {
        if (subscription->topic == topic) {
            int i= 0;
            while ((i < OS_NUMBER_OF_SUBS) &&  (subscription->actions[i] != NULL)) {
                os_DoWith(subscription->actions[i++], context);
            }
            return;
        }
        subscription = subscription->next;
    }

    // Topic not found, do nothing
}

