#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

os_subscription_t *os_subFreeList;
uint32_t           os_subHighWaterMark;
uint32_t           os_subInUseCount;
os_subscription_t  os_subPool[OS_MAX_SUBSCRIPTIONS];

#define TEST_UNSET  0xDEADBEEF

static void setUp(void)
{
    Test_Init();
}

static void test_ResetsInUseCount(void)
{
    setUp();
    os_subInUseCount = TEST_UNSET;

    os_SubAllocInit();

    Assert_Equals(0, os_subInUseCount);
}

static void test_ResetsHighWaterMark(void)
{
    setUp();
    os_subHighWaterMark = TEST_UNSET;

    os_SubAllocInit();

    Assert_Equals(0, os_subHighWaterMark);
}

static void test_SetsFreeListHeadToFirstPoolSlot(void)
{
    setUp();
    os_subFreeList = NULL;

    os_SubAllocInit();

    Assert_Equals(&os_subPool[0], os_subFreeList);
}

static void test_FreeListLinksSlotsEndingInNull(void)
{
    setUp();

    os_SubAllocInit();

    Assert_Equals(&os_subPool[1], os_subPool[0].next);
    Assert_IsNull(os_subPool[OS_MAX_SUBSCRIPTIONS - 1].next);
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
