#include "os/os_p.h"

extern uint32_t os_subHighWaterMark;

void os_SubHighWaterReset(void)
{
    os_subHighWaterMark = 0;
}
