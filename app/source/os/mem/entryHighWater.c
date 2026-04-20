#include "os/os_p.h"

extern uint32_t os_entryHighWaterMark;

uint32_t os_EntryHighWater(void)
{
    return os_entryHighWaterMark;
}
