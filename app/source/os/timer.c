#include "os/os_p.h"
#include <stdlib.h>

extern os_tqEntry_t *os_tQueue;

extern uint32_t SystemCoreClock;

void os_TimerInit(arm_SCS_t *system, uint32_t sysTicksPerOsTick) {
    os_tQueue = NULL;

    system->systick.LOAD = sysTicksPerOsTick;
    system->scb.SHPR3 |= 0xFF000000;  // TODO: magic numbers?
    system->systick.VAL = 0;
    system->systick.CTRL = 7; // TODO: magic numbes
}

// Assumes interrupts disabled.
// Assumes ticks > 0.
void os_TimerAdd(os_tqEntry_t *entry)
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
        os_tqEntry_t *cursor = os_tQueue;
        entry->ticks -= cursor->ticks;
        while ((cursor->next != NULL) && (entry->ticks >= cursor->next->ticks)) {
            cursor = cursor->next;

            // Adjust entry ticks to account for the items that come before us
            entry->ticks -= cursor->ticks;
        }

        // Skip past items in the list with the same ticks, we goto the end of the line
        while ((cursor->next != NULL) && (cursor->next->ticks == 0)) {
            cursor = cursor->next;
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
