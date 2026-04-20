#include "hal/Critical.h"
#include "os/os_p.h"

extern os_entry_t *os_entryFreeList;
extern uint32_t    os_entryInUseCount;

void os_EntryFree(os_entry_t *entry)
{
    if (entry == NULL) {
        os_Fail(OS_FAIL_ENTRY_FREE);
        return;
    }
    hal_CriticalBegin();
    entry->next = os_entryFreeList;
    os_entryFreeList = entry;
    os_entryInUseCount--;
    hal_CriticalEnd();
}
