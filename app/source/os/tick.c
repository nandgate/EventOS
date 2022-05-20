#include "os/os_p.h"
#include <stdlib.h>

extern os_tqEntry_t *os_tQueue;

void SysTick_Handler(void)
{
    // If queue is empty skip.  TODO: Add logic to disable IRQ when queue is empty
    if (os_tQueue != NULL) {
        // Tick the head of the queue
        os_tQueue->ticks--;

        // Entry expired?
        if (os_tQueue->ticks == 0) {
            do {
                // Time expired, add an entry to the FIFO of things to do
                os_actionEntry_t *actionEntry = os_MemAlloc(sizeof(os_actionEntry_t));
                while(actionEntry == NULL); // TODO: Error handling for when malloc fails, NULL return- out of memory

                actionEntry->action = os_tQueue->action;
                actionEntry->ctx = os_tQueue->ctx;
                os_FifoAdd(actionEntry);

                // Advance to the next item in the queue, it may have expired too...
                os_tqEntry_t *tQueueOld = os_tQueue;
                os_tQueue = os_tQueue->next;
                os_MemFree(tQueueOld);
            } while ((os_tQueue != NULL) && (os_tQueue->ticks == 0));
        }
    }
}