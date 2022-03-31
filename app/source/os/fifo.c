#include "os/os_p.h"
#include <stdlib.h>

extern os_actionFifo_t os_actionFifo;

void os_FifoAdd(os_actionEntry_t *fifoEntry)
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

os_actionEntry_t *os_FifoRemove(void)
{
    os_actionEntry_t *result = os_actionFifo.os_fifoHead;
    if (result != NULL) {
        os_actionFifo.os_fifoHead = os_actionFifo.os_fifoHead->next;
        if (os_actionFifo.os_fifoHead == NULL) {
            os_actionFifo.os_fifoTail = NULL;
        }
    }
    return result;
}
