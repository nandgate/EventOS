#include "os/os_p.h"

os_entry_t  os_entryPool[OS_MAX_ENTRIES];
os_entry_t *os_entryFreeList;
uint32_t    os_entryInUseCount;
uint32_t    os_entryHighWaterMark;

void os_EntryAllocInit(void)
{
    os_entryFreeList = &os_entryPool[0];
    for (uint32_t i = 0; i < OS_MAX_ENTRIES - 1; i++) {
        os_entryPool[i].next = &os_entryPool[i + 1];
    }
    os_entryPool[OS_MAX_ENTRIES - 1].next = NULL;
    os_entryInUseCount = 0;
    os_entryHighWaterMark = 0;
}
