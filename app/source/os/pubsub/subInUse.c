#include "os/os_p.h"

extern uint32_t os_subInUseCount;

uint32_t os_SubInUse(void)
{
    return os_subInUseCount;
}
