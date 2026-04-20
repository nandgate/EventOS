#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

Mock_Void(hal_CriticalBegin);
Mock_Void(hal_CriticalEnd);
Mock_Void1(os_Fail, os_fail_t);

os_subscription_t *os_subFreeList;
uint32_t           os_subInUseCount;

static void setUp(void)
{
    Test_Init();
    Mock_Reset(hal_CriticalBegin);
    Mock_Reset(hal_CriticalEnd);
    Mock_Reset(os_Fail);
}

static void test_NewSubBecomesFreeListHead(void)
{
    setUp();
    os_subscription_t sub;
    os_subFreeList = NULL;

    os_SubFree(&sub);

    Assert_Equals(&sub, os_subFreeList);
}

static void test_LinksSubToPreviousHead(void)
{
    setUp();
    os_subscription_t sub;
    os_subscription_t oldHead;
    os_subFreeList = &oldHead;

    os_SubFree(&sub);

    Assert_Equals(&oldHead, sub.next);
}

static void test_DecrementsInUseCount(void)
{
    setUp();
    os_subscription_t sub;
    os_subFreeList = NULL;
    os_subInUseCount = 7;

    os_SubFree(&sub);

    Assert_Equals(6, os_subInUseCount);
}

static void test_Null_CallsFail(void)
{
    setUp();

    os_SubFree(NULL);

    Assert_CalledOnce(os_Fail);
    Assert_Called1(os_Fail, OS_FAIL_SUB_FREE);
}

static void test_WrapsInCriticalSection(void)
{
    setUp();
    os_subscription_t sub;
    os_subFreeList = NULL;

    os_SubFree(&sub);

    Assert_CalledOnce(hal_CriticalBegin);
    Assert_CalledOnce(hal_CriticalEnd);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_NewSubBecomesFreeListHead();
    test_LinksSubToPreviousHead();
    test_DecrementsInUseCount();
    test_Null_CallsFail();
    test_WrapsInCriticalSection();

    Assert_Save();
    return 0;
}
