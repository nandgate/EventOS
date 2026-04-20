#include "os/os_p.h"

extern uint32_t os_entryInUseCount;

uint32_t os_EntryInUse(void)
{
    return os_entryInUseCount;
}
