#include "os/os_p.h"
#include <stdlib.h>

void os_MemInit(void)
{
}

void *os_MemAlloc(uint32_t size)
{
    void * result;

    // Use the system malloc for now (the horror), to be replaced with our own memory management with known properties
    result = malloc(size);
    return result;
}

void os_MemFree(void *mem)
{
    // Use the system malloc for now (it probably works), to be replaced with owr own memory management
    free(mem);
}
