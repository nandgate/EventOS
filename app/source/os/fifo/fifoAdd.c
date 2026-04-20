#include "os/os_p.h"

extern os_actionFifo_t os_actionFifo;

// Called from ISR context (SysTick_Handler). Must not allocate or block.
void os_FifoAdd(os_entry_t *fifoEntry)
{
    if (fifoEntry != NULL) {
        if (os_actionFifo.os_fifoTail == NULL) {
            os_actionFifo.os_fifoHead = fifoEntry;
        } else {
            os_actionFifo.os_fifoTail->next = fifoEntry;
        }
        os_actionFifo.os_fifoTail = fifoEntry;
        fifoEntry->next = NULL;
    }
}
