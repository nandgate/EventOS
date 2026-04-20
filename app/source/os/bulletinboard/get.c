#include "hal/Critical.h"
#include "os/os_p.h"

extern os_actionFifo_t os_actionFifo;
extern os_entry_t *os_tQueue;
extern os_ctx_t *os_heldCtx;

static os_entry_t *takeFromTimerQueue(uint32_t key)
{
    if (os_tQueue == NULL) {
        return NULL;
    }
    if (os_tQueue->key == key) {
        os_entry_t *found = os_tQueue;
        os_tQueue = found->next;
        if (os_tQueue != NULL) {
            os_tQueue->ticks += found->ticks;
        }
        return found;
    }
    os_entry_t *cursor = os_tQueue;
    while (cursor->next != NULL) {
        if (cursor->next->key == key) {
            os_entry_t *found = cursor->next;
            cursor->next = found->next;
            if (cursor->next != NULL) {
                cursor->next->ticks += found->ticks;
            }
            return found;
        }
        cursor = cursor->next;
    }
    return NULL;
}

static os_entry_t *takeFromFifo(uint32_t key)
{
    if (os_actionFifo.os_fifoHead == NULL) {
        return NULL;
    }
    if (os_actionFifo.os_fifoHead->key == key) {
        os_entry_t *found = os_actionFifo.os_fifoHead;
        os_actionFifo.os_fifoHead = found->next;
        if (os_actionFifo.os_fifoHead == NULL) {
            os_actionFifo.os_fifoTail = NULL;
        }
        return found;
    }
    os_entry_t *cursor = os_actionFifo.os_fifoHead;
    while (cursor->next != NULL) {
        if (cursor->next->key == key) {
            os_entry_t *found = cursor->next;
            cursor->next = found->next;
            if (found == os_actionFifo.os_fifoTail) {
                os_actionFifo.os_fifoTail = cursor;
            }
            return found;
        }
        cursor = cursor->next;
    }
    return NULL;
}

os_context_t os_Get(uint32_t key)
{
    if (key == OS_NO_KEY) {
        return NULL;
    }

    hal_CriticalBegin();
    os_entry_t *found = takeFromTimerQueue(key);
    if (found == NULL) {
        found = takeFromFifo(key);
    }
    hal_CriticalEnd();

    if (found == NULL) {
        return NULL;
    }

    // Transfer context ownership to the getting action. Prepend onto the
    // action's held-list; os_Exec walks and releases after the action returns.
    os_ctx_t *ctx = found->ctx;
    if (ctx != NULL) {
        ctx->next = os_heldCtx;
        os_heldCtx = ctx;
    }

    os_EntryFree(found);

    return ctx != NULL ? ctx->data : NULL;
}
