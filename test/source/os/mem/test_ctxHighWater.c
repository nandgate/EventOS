#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

uint32_t os_ctxSmallHighWaterMark;
uint32_t os_ctxLargeHighWaterMark;

static void setUp(void)
{
    Test_Init();
    os_ctxSmallHighWaterMark = 0;
    os_ctxLargeHighWaterMark = 0;
}

static void test_ReturnsSmallMark(void)
{
    setUp();
    os_ctxSmallHighWaterMark = 7;
    os_ctxLargeHighWaterMark = 11;

    uint32_t result = os_CtxHighWater(OS_CTX_BUCKET_SMALL);

    Assert_Equals(7, result);
}

static void test_ReturnsLargeMark(void)
{
    setUp();
    os_ctxSmallHighWaterMark = 7;
    os_ctxLargeHighWaterMark = 11;

    uint32_t result = os_CtxHighWater(OS_CTX_BUCKET_LARGE);

    Assert_Equals(11, result);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_ReturnsSmallMark();
    test_ReturnsLargeMark();

    Assert_Save();
    return 0;
}
