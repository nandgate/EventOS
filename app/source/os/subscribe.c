#include "os/os_p.h"
#include <stdlib.h>

extern os_subscription_t *os_subscriptions;

void os_Subscribe(uint32_t topic, os_action_t action) {
    os_subscription_t *sub;

    // See if the topic all ready has a subscription
    for(sub = os_subscriptions; sub != NULL; sub= sub->next) {
        if (sub->topic == topic) {
            // Find an empty slot and fill it
            for(int i= 0; i < OS_NUMBER_OF_SUBS; i++) {
                if (sub->actions[i] == NULL) {
                    sub->actions[i] = action;
                    return;
                }
            }
            // No slot found- ignore it (for now....)
            // TODO: Deal with too many subscribers
            return;
        }
    }

    // Create a new subscription for the topic and add it to the list
    sub = os_MemAlloc(sizeof(os_subscription_t));

    sub->topic = topic;
    sub->actions[0] = action;
    for(int i= 1; i < OS_NUMBER_OF_SUBS; i++) {
        sub->actions[i] = NULL;
    }
    sub->next = os_subscriptions;
    os_subscriptions = sub;
}

