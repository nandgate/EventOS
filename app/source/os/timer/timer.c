#include "os/os_p.h"

extern os_entry_t *os_tQueue;

void os_TimerInit(void) {
    os_tQueue = NULL;
}
