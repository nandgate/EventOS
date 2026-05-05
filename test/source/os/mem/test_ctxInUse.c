#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

uint32_t os_ctxSmallInUseCount;
uint32_t os_ctxLargeInUseCount;

static void setUp(void)
{
    Test_Init();
    os_ctxSmallInUseCount = 0;
    os_ctxLargeInUseCount = 0;
}

static void test_ReturnsSmallCount(void)
{
    setUp();
    os_ctxSmallInUseCount = 7;
    os_ctxLargeInUseCount = 11;

    uint32_t result = os_CtxInUse(OS_CTX_BUCKET_SMALL);

    Assert_Equals(7, result);
}

static void test_ReturnsLargeCount(void)
{
    setUp();
    os_ctxSmallInUseCount = 7;
    os_ctxLargeInUseCount = 11;

    uint32_t result = os_CtxInUse(OS_CTX_BUCKET_LARGE);

    Assert_Equals(11, result);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_ReturnsSmallCount();
    test_ReturnsLargeCount();

    Assert_Save();
    return 0;
}
