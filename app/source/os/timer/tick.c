#include "os/os_p.h"

extern os_entry_t *os_tQueue;

void os_Tick(void)
{
    if (os_tQueue != NULL) {
        // Tick the head of the queue
        os_tQueue->ticks--;

        // Entry expired?
        if (os_tQueue->ticks == 0) {
            do {
                // Unlink the expired entry from the timer queue and hand it to the
                // FIFO. No allocation or copy — the same entry moves between lists.
                os_entry_t *expired = os_tQueue;
                os_tQueue = os_tQueue->next;
                os_FifoAdd(expired);
            } while ((os_tQueue != NULL) && (os_tQueue->ticks == 0));
        }
    }
}
