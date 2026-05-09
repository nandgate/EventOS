#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

os_ctx_t test_contextEntry;

Mock_Void1(os_CtxFree, os_ctx_t *);

static void setUp(void)
{
    Test_Init();
    Mock_Reset(os_CtxFree);
}

static void test_NullIsNoop(void)
{
    setUp();

    os_ContextRelease(NULL);

    Assert_NotCalled(os_CtxFree);
}

static void test_StillInUse(void)
{
    setUp();
    test_contextEntry.count = 2;

    os_ContextRelease(&test_contextEntry);

    Assert_Equals(1, test_contextEntry.count);
    Assert_NotCalled(os_CtxFree);
}

static void test_LastUse(void)
{
    setUp();
    test_contextEntry.count = 1;

    os_ContextRelease(&test_contextEntry);

    Assert_Equals(0, test_contextEntry.count);
    Assert_CalledOnce(os_CtxFree);
    Assert_Called1(os_CtxFree, &test_contextEntry);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_NullIsNoop();
    test_StillInUse();
    test_LastUse();

    Assert_Save();
    return 0;
}
