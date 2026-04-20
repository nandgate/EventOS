#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

Mock_Void(hal_CriticalBegin);
Mock_Void(hal_CriticalEnd);
Mock_Void1(os_FifoCancel, uint32_t);
Mock_Void1(os_TimerRemove, uint32_t);

#define TEST_KEY 0xCAFE

static void setUp(void) {
    Test_Init();
    Mock_Reset(hal_CriticalBegin);
    Mock_Reset(hal_CriticalEnd);
    Mock_Reset(os_FifoCancel);
    Mock_Reset(os_TimerRemove);
}

static void test_DelegatesToTimerRemove(void) {
    setUp();

    os_CancelPending(TEST_KEY);

    Assert_CalledOnce(os_TimerRemove);
    Assert_Called1(os_TimerRemove, TEST_KEY);
}

static void test_DelegatesToFifoCancel(void) {
    setUp();

    os_CancelPending(TEST_KEY);

    Assert_CalledOnce(os_FifoCancel);
    Assert_Called1(os_FifoCancel, TEST_KEY);
}

int main(int argc, char **argv) {
    Assert_Init();

    test_DelegatesToTimerRemove();
    test_DelegatesToFifoCancel();

    Assert_Save();
    return 0;
}
