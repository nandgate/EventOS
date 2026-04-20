#include "os/os_p.h"

extern os_actionFifo_t os_actionFifo;

void os_FifoCancel(uint32_t key) {
    if (key == OS_NO_KEY) {
        return;
    }

    // Remove matching entries at the head
    while ((os_actionFifo.os_fifoHead != NULL) && (os_actionFifo.os_fifoHead->key == key)) {
        os_entry_t *removed = os_actionFifo.os_fifoHead;
        os_actionFifo.os_fifoHead = removed->next;
        if (os_actionFifo.os_fifoHead == NULL) {
            os_actionFifo.os_fifoTail = NULL;
        }
        os_ContextRelease(removed->ctx);
        os_EntryFree(removed);
    }

    // Walk the rest removing matching entries
    os_entry_t *cursor = os_actionFifo.os_fifoHead;
    while ((cursor != NULL) && (cursor->next != NULL)) {
        if (cursor->next->key == key) {
            os_entry_t *removed = cursor->next;
            cursor->next = removed->next;
            if (removed == os_actionFifo.os_fifoTail) {
                os_actionFifo.os_fifoTail = cursor;
            }
            os_ContextRelease(removed->ctx);
            os_EntryFree(removed);
        } else {
            cursor = cursor->next;
        }
    }
}
