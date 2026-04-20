#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

Mock_Void(hal_CriticalBegin);
Mock_Void(hal_CriticalEnd);

os_subscription_t *os_subFreeList;
uint32_t           os_subInUseCount;
uint32_t           os_subHighWaterMark;

static void setUp(void)
{
    Test_Init();
    Mock_Reset(hal_CriticalBegin);
    Mock_Reset(hal_CriticalEnd);
}

static void test_EmptyFreeList_ReturnsNull(void)
{
    setUp();
    os_subFreeList = NULL;

    os_subscription_t *result = os_SubAlloc();

    Assert_IsNull(result);
}

static void test_ReturnsFreeListHead(void)
{
    setUp();
    os_subscription_t slot;
    os_subFreeList = &slot;

    os_subscription_t *result = os_SubAlloc();

    Assert_Equals(&slot, result);
}

static void test_AdvancesFreeListHead(void)
{
    setUp();
    os_subscription_t slot1, slot2;
    slot1.next = &slot2;
    os_subFreeList = &slot1;

    os_SubAlloc();

    Assert_Equals(&slot2, os_subFreeList);
}

static void test_IncrementsInUseCount(void)
{
    setUp();
    os_subscription_t slot;
    os_subFreeList = &slot;
    os_subInUseCount = 5;

    os_SubAlloc();

    Assert_Equals(6, os_subInUseCount);
}

static void test_RaisesHighWaterWhenInUseExceedsIt(void)
{
    setUp();
    os_subscription_t slot;
    os_subFreeList = &slot;
    os_subInUseCount = 5;
    os_subHighWaterMark = 5;

    os_SubAlloc();

    Assert_Equals(6, os_subHighWaterMark);
}

static void test_DoesNotLowerHighWater(void)
{
    setUp();
    os_subscription_t slot;
    os_subFreeList = &slot;
    os_subInUseCount = 3;
    os_subHighWaterMark = 10;

    os_SubAlloc();

    Assert_Equals(10, os_subHighWaterMark);
}

static void test_WrapsInCriticalSection(void)
{
    setUp();
    os_subscription_t slot;
    os_subFreeList = &slot;

    os_SubAlloc();

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
