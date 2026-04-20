#include "hal/Critical.h"
#include "os/os_p.h"

extern os_ctx_t *os_heldCtx;

void os_Exec(void) {
    hal_CriticalBegin();
    os_entry_t *fifoEntry = os_FifoRemove();
    hal_CriticalEnd();
    if (fifoEntry != NULL) {
        os_ctx_t *ctx = fifoEntry->ctx;
        os_ActionBegin(fifoEntry->action);
        fifoEntry->action(ctx != NULL ? ctx->data : NULL);
        os_ActionEnd(fifoEntry->action);
        // Release every context the action held via os_Get.
        while (os_heldCtx != NULL) {
            os_ctx_t *held = os_heldCtx;
            os_heldCtx = held->next;
            held->next = NULL;
            os_ContextRelease(held);
        }
        if (ctx != NULL) {
            os_ContextRelease(ctx);
        }
        os_EntryFree(fifoEntry);
    }
}
