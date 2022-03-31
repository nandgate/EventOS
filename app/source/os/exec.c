#include "os/os_p.h"
//#include "cmsis_gcc.h"  // must come after os_p.h
#include <stdlib.h>

void os_Exec(void) {
    __disable_irq();
    os_actionEntry_t *fifoEntry = os_FifoRemove();
    if (fifoEntry != NULL) {
        __enable_irq();
        fifoEntry->action(fifoEntry->ctx->data);
        __disable_irq();
        os_ContextUse(fifoEntry->ctx);
        os_MemFree(fifoEntry);
    }
    __enable_irq();
}
