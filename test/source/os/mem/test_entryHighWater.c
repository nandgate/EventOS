#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

uint32_t os_entryHighWaterMark;

static void setUp(void)
{
    Test_Init();
}

static void test_ReturnsMark(void)
{
    setUp();
    os_entryHighWaterMark = 42;

    uint32_t result = os_EntryHighWater();

    Assert_Equals(42, result);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_ReturnsMark();

    Assert_Save();
    return 0;
}
