#include "hal/Critical.h"
#include "os/os_p.h"

extern os_entry_t *os_entryFreeList;
extern uint32_t    os_entryInUseCount;
extern uint32_t    os_entryHighWaterMark;

os_entry_t *os_EntryAlloc(void)
{
    hal_CriticalBegin();
    os_entry_t *slot = os_entryFreeList;
    if (slot != NULL) {
        os_entryFreeList = slot->next;
        os_entryInUseCount++;
        if (os_entryInUseCount > os_entryHighWaterMark) {
            os_entryHighWaterMark = os_entryInUseCount;
        }
    }
    hal_CriticalEnd();
    return slot;
}
