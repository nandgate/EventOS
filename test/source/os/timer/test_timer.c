#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

#define TEST_UNSET 0x42

os_entry_t *os_tQueue;

static void setUp(void)
{
    Test_Init();
    os_tQueue = (os_entry_t *) TEST_UNSET;
}

static void test_Init(void)
{
    setUp();

    os_TimerInit();

    Assert_Equals(NULL, os_tQueue);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_Init();

    Assert_Save();
    return 0;
}
