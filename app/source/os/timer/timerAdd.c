#include "os/os_p.h"

extern os_entry_t *os_tQueue;

// Assumes interrupts disabled.
// Assumes ticks > 0.
void os_TimerAdd(os_entry_t *entry)
{
    if (os_tQueue == NULL)
    {
        // Empty list, just add the entry
        os_tQueue = entry;
        entry->next = NULL;
    }
    else if (entry->ticks < os_tQueue->ticks) {
        // Insert Before, add to the front of the list
        entry->next = os_tQueue;
        os_tQueue = entry;
        os_tQueue->next->ticks -= entry->ticks;
    }
    else {
        // Walk the list looking for the place to insert the entry
        os_entry_t *cursor = os_tQueue;
        entry->ticks -= cursor->ticks;
        while ((cursor->next != NULL) && (entry->ticks >= cursor->next->ticks)) {
            cursor = cursor->next;

            // Adjust entry ticks to account for the items that come before us
            entry->ticks -= cursor->ticks;
        }

        // Adjust the ticks in the next node (when there is one) to account for this node
        if (cursor->next != NULL) {
            cursor->next->ticks -= entry->ticks;
        }

        // Actually insert in to the list
        entry->next = cursor->next;
        cursor->next = entry;
    }
}
