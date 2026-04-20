#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

Mock_Void(hal_CriticalBegin);
Mock_Void(hal_CriticalEnd);

os_entry_t *os_entryFreeList;
uint32_t    os_entryInUseCount;
uint32_t    os_entryHighWaterMark;

static void setUp(void)
{
    Test_Init();
    Mock_Reset(hal_CriticalBegin);
    Mock_Reset(hal_CriticalEnd);
}

static void test_EmptyFreeList_ReturnsNull(void)
{
    setUp();
    os_entryFreeList = NULL;

    os_entry_t *result = os_EntryAlloc();

    Assert_IsNull(result);
}

static void test_ReturnsFreeListHead(void)
{
    setUp();
    os_entry_t slot;
    os_entryFreeList = &slot;

    os_entry_t *result = os_EntryAlloc();

    Assert_Equals(&slot, result);
}

static void test_AdvancesFreeListHead(void)
{
    setUp();
    os_entry_t slot1, slot2;
    slot1.next = &slot2;
    os_entryFreeList = &slot1;

    os_EntryAlloc();

    Assert_Equals(&slot2, os_entryFreeList);
}

static void test_IncrementsInUseCount(void)
{
    setUp();
    os_entry_t slot;
    os_entryFreeList = &slot;
    os_entryInUseCount = 5;

    os_EntryAlloc();

    Assert_Equals(6, os_entryInUseCount);
}

static void test_RaisesHighWaterWhenInUseExceedsIt(void)
{
    setUp();
    os_entry_t slot;
    os_entryFreeList = &slot;
    os_entryInUseCount = 5;
    os_entryHighWaterMark = 5;

    os_EntryAlloc();

    Assert_Equals(6, os_entryHighWaterMark);
}

static void test_DoesNotLowerHighWater(void)
{
    setUp();
    os_entry_t slot;
    os_entryFreeList = &slot;
    os_entryInUseCount = 3;
    os_entryHighWaterMark = 10;

    os_EntryAlloc();

    Assert_Equals(10, os_entryHighWaterMark);
}

static void test_WrapsInCriticalSection(void)
{
    setUp();
    os_entry_t slot;
    os_entryFreeList = &slot;

    os_EntryAlloc();

    Assert_CalledOnce(hal_CriticalBegin);
    Assert_CalledOnce(hal_CriticalEnd);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_EmptyFreeList_ReturnsNull();
    test_ReturnsFreeListHead();
    test_AdvancesFreeListHead();
    test_IncrementsInUseCount();
    test_RaisesHighWaterWhenInUseExceedsIt();
    test_DoesNotLowerHighWater();
    test_WrapsInCriticalSection();

    Assert_Save();
    return 0;
}
