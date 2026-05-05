#include "os/os_p.h"

extern uint32_t os_entryHighWaterMark;

void os_EntryHighWaterReset(void)
{
    os_entryHighWaterMark = 0;
}
