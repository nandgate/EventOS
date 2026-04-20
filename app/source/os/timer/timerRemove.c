#include "os/os_p.h"

extern os_entry_t *os_tQueue;

void os_TimerRemove(uint32_t key) {
    if (key == OS_NO_KEY) {
        return;
    }

    // Remove matching entries at the head
    while ((os_tQueue != NULL) && (os_tQueue->key == key)) {
        os_entry_t *removed = os_tQueue;
        os_tQueue = os_tQueue->next;
        if (os_tQueue != NULL) {
            os_tQueue->ticks += removed->ticks;
        }
        os_ContextRelease(removed->ctx);
        os_EntryFree(removed);
    }

    // Walk the rest of the list removing matching entries
    os_entry_t *cursor = os_tQueue;
    while ((cursor != NULL) && (cursor->next != NULL)) {
        if (cursor->next->key == key) {
            os_entry_t *removed = cursor->next;
            cursor->next = removed->next;
            if (cursor->next != NULL) {
                cursor->next->ticks += removed->ticks;
            }
            os_ContextRelease(removed->ctx);
            os_EntryFree(removed);
        } else {
            cursor = cursor->next;
        }
    }
}
