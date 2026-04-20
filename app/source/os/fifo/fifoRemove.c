#include "os/os_p.h"

extern os_actionFifo_t os_actionFifo;

os_entry_t *os_FifoRemove(void)
{
    os_entry_t *result = os_actionFifo.os_fifoHead;
    if (result != NULL) {
        os_actionFifo.os_fifoHead = os_actionFifo.os_fifoHead->next;
        if (os_actionFifo.os_fifoHead == NULL) {
            os_actionFifo.os_fifoTail = NULL;
        }
    }
    return result;
}
