#include "os/os_p.h"
#include <stdbool.h>
#include <stdlib.h>

extern os_subscription_t *os_subscriptions;

static bool os_UnsubAll(uint32_t topic, os_subscription_t *cursor, os_subscription_t *sub) {
    // Does the topic match?
    if (sub->topic == topic) {
        // yes, remove the subscription from the list.
        cursor->next = sub->next;
        os_MemFree(sub);
        return true;
    }
    // topic not found, return false
    return false;
}

void os_UnsubscribeAll(uint32_t topic) {
    if (os_subscriptions != NULL) {
        // check the first node as a special case.
        if (!os_UnsubAll(topic, (os_subscription_t *) &os_subscriptions, os_subscriptions)) {
            // Not the fist node, walk the list looking for a matching topic.
            for (os_subscription_t *cursor = os_subscriptions; cursor->next != NULL; cursor = cursor->next) {
                if (os_UnsubAll(topic, cursor, cursor->next)) {
                    // topic was in the node, we are finished
                    break;
                }
            }
        }
    }
}