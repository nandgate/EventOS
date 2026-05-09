#include "os/os_p.h"

extern os_entry_t *os_entryFreeList;
extern uint32_t    os_entryHighWaterMark;
extern uint32_t    os_entryInUseCount;
extern os_entry_t  os_entryPool[OS_MAX_ENTRIES];

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
