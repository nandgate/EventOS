#include "os/os_p.h"
#include "UnitTest.h"

#define TEST_UNSET 42

Mock_Vars(3);

uint32_t os_entryHighWaterMark;

static void setUp(void)
{
    Test_Init();
    os_entryHighWaterMark = TEST_UNSET;
}

static void test_ClearsHighWaterMark(void)
{
    setUp();

    os_EntryHighWaterReset();

    Assert_Equals(0, os_entryHighWaterMark);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_ClearsHighWaterMark();

    Assert_Save();
    return 0;
}
