#include "os/os_p.h"
#include "UnitTest.h"

#define TEST_UNSET 42

Mock_Vars(3);

uint32_t os_ctxSmallHighWaterMark;
uint32_t os_ctxLargeHighWaterMark;

static void setUp(void)
{
    Test_Init();
    os_ctxSmallHighWaterMark = TEST_UNSET;
    os_ctxLargeHighWaterMark = TEST_UNSET;
}

static void test_SmallClearsSmallOnly(void)
{
    setUp();

    os_CtxHighWaterReset(OS_CTX_BUCKET_SMALL);

    Assert_Equals(0, os_ctxSmallHighWaterMark);
    Assert_Equals(TEST_UNSET, os_ctxLargeHighWaterMark);
}

static void test_LargeClearsLargeOnly(void)
{
    setUp();

    os_CtxHighWaterReset(OS_CTX_BUCKET_LARGE);

    Assert_Equals(TEST_UNSET, os_ctxSmallHighWaterMark);
    Assert_Equals(0, os_ctxLargeHighWaterMark);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_SmallClearsSmallOnly();
    test_LargeClearsLargeOnly();

    Assert_Save();
    return 0;
}
