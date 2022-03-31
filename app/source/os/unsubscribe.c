#include "os/os_p.h"
#include <stdbool.h>
#include <stdlib.h>

extern os_subscription_t *os_subscriptions;

static bool os_Unsub(uint32_t topic, os_action_t action, os_subscription_t *cursor, os_subscription_t *sub) {
    // Does the topic match?
    if (sub->topic == topic) {
        // yes, find a matching action
        for (int i = 0; i < OS_NUMBER_OF_SUBS; i++) {
            if (sub->actions[i] == action) {
                // remove this action by sliding all actions beyond this one up in the array.
                while (i < OS_NUMBER_OF_SUBS - 1) {
                    sub->actions[i] = sub->actions[i + 1];
                    i++;
                }
                sub->actions[OS_NUMBER_OF_SUBS-1] = NULL;

                // is this the last action in the subscription?
                if (sub->actions[0] == NULL) {
                    // yes, remove the subscription from the list.
                    cursor->next = sub->next;
                    os_MemFree(sub);
                }
                break;
            }
        }
        // topic was found, return true
        return true;
    }
    // topic not found, return false
    return false;
}

void os_Unsubscribe(uint32_t topic, os_action_t action) {
    if (os_subscriptions != NULL) {
        // check the first node as a special case.
        if (!os_Unsub(topic, action, (os_subscription_t *) &os_subscriptions, os_subscriptions)) {
            // Not the fist node, walk the list looking for a matching topic.
            for (os_subscription_t *cursor = os_subscriptions; cursor->next != NULL; cursor = cursor->next) {
                if (os_Unsub(topic, action, cursor, cursor->next)) {
                    // topic was in the node, we are finished
                    break;
                }
            }
        }
    }
}