#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

uint32_t os_entryInUseCount;

static void setUp(void)
{
    Test_Init();
}

static void test_ReturnsCurrentCount(void)
{
    setUp();
    os_entryInUseCount = 42;

    uint32_t result = os_EntryInUse();

    Assert_Equals(42, result);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_ReturnsCurrentCount();

    Assert_Save();
    return 0;
}
