#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

os_entry_t  os_entryPool[OS_MAX_ENTRIES];
extern os_entry_t *os_entryFreeList;
extern uint32_t    os_entryInUseCount;
extern uint32_t    os_entryHighWaterMark;

#define TEST_UNSET  0xDEADBEEF

static void setUp(void)
{
    Test_Init();
}

static void test_ResetsInUseCount(void)
{
    setUp();
    os_entryInUseCount = TEST_UNSET;

    os_EntryAllocInit();

    Assert_Equals(0, os_entryInUseCount);
}

static void test_ResetsHighWaterMark(void)
{
    setUp();
    os_entryHighWaterMark = TEST_UNSET;

    os_EntryAllocInit();

    Assert_Equals(0, os_entryHighWaterMark);
}

static void test_SetsFreeListHeadToFirstPoolSlot(void)
{
    setUp();
    os_entryFreeList = NULL;

    os_EntryAllocInit();

    Assert_Equals(&os_entryPool[0], os_entryFreeList);
}

static void test_FreeListLinksSlotsEndingInNull(void)
{
    setUp();

    os_EntryAllocInit();

    Assert_Equals(&os_entryPool[1], os_entryPool[0].next);
    Assert_IsNull(os_entryPool[OS_MAX_ENTRIES - 1].next);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_ResetsInUseCount();
    test_ResetsHighWaterMark();
    test_SetsFreeListHeadToFirstPoolSlot();
    test_FreeListLinksSlotsEndingInNull();

    Assert_Save();
    return 0;
}
