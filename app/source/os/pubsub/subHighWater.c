#include "os/os_p.h"

extern uint32_t os_subHighWaterMark;

uint32_t os_SubHighWater(void)
{
    return os_subHighWaterMark;
}
