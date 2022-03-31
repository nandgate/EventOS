#include "os/os_p.h"
#include <stdlib.h>

void os_MemInit(void)
{
}

void *os_MemAlloc(uint32_t size)
{
    // Use the system malloc for now (the horror), to be replaced with our own memory management with known properties
    return malloc(size);
}

void os_MemFree(void *mem)
{
    // Use the system malloc for now (it probably works), to be replaced with owr own memory management
    free(mem);
}
