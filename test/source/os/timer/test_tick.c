#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

Mock_Void1(os_Fail, os_fail_t);
Mock_Void1(os_FifoAdd, os_entry_t *);
Mock_Void1(test_Action, os_context_t);

#define TEST_TICKS          2
#define TEST_EXPIRED_TICKS  1
#define TEST_SAME_TIME      0

os_ctx_t test_ctx;
os_entry_t *os_tQueue;
os_entry_t test_tqEntry1;
os_entry_t test_tqEntry2;
os_entry_t test_tqEntry3;

static void setUp(void)
{
    Test_Init();
    Mock_Reset(os_Fail);
    Mock_Reset(os_FifoAdd);
    Mock_Reset(test_Action);

    os_tQueue = &test_tqEntry1;
    test_tqEntry1.ticks = TEST_TICKS;
    test_tqEntry1.action = &test_Action;
    test_tqEntry1.ctx = &test_ctx;
    test_tqEntry1.next = &test_tqEntry2;

    test_tqEntry2.ticks = TEST_TICKS;
    test_tqEntry2.next = &test_tqEntry3;

    test_tqEntry3.ticks = TEST_TICKS;
    test_tqEntry3.next = NULL;
}

static void test_SysTick_Empty(void)
{
    setUp();
    os_tQueue = NULL;

    os_Tick();

    Assert_NotCalled(os_FifoAdd);
}

static void test_SysTick_TimerNotExpired(void) {
    setUp();

    os_Tick();

    Assert_Equals(TEST_TICKS-1, test_tqEntry1.ticks);
}

static void test_SysTick_TimerExpired_EnqueuesExpiredEntry(void) {
    setUp();
    test_tqEntry1.ticks = TEST_EXPIRED_TICKS;

    os_Tick();

    Assert_CalledOnce(os_FifoAdd);
    Assert_Called1(os_FifoAdd, &test_tqEntry1);
}

static void test_SysTick_TimerExpired_NoNext(void)
{
    setUp();
    test_tqEntry1.ticks = TEST_EXPIRED_TICKS;
    test_tqEntry1.next = NULL;

    os_Tick();

    Assert_Equals(NULL, os_tQueue);
}

static void test_SysTick_TimerExpired_TickNext(void)
{
    setUp();
    test_tqEntry1.ticks = TEST_EXPIRED_TICKS;

    os_Tick();

    Assert_Equals(&test_tqEntry2, os_tQueue);
}

static void test_SysTick_MoreThanOneExpired(void)
{
    setUp();
    os_tQueue = &test_tqEntry1;
    test_tqEntry1.ticks = TEST_EXPIRED_TICKS;
    test_tqEntry2.ticks = TEST_SAME_TIME;

    os_Tick();

    Assert_CallCount(2, os_FifoAdd);
    Assert_CalledFirst1(os_FifoAdd, &test_tqEntry1);
    Assert_CalledLast1(os_FifoAdd, &test_tqEntry2);
    Assert_Equals(&test_tqEntry3, os_tQueue);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_SysTick_Empty();
    test_SysTick_TimerNotExpired();
    test_SysTick_TimerExpired_EnqueuesExpiredEntry();
    test_SysTick_TimerExpired_NoNext();
    test_SysTick_TimerExpired_TickNext();
    test_SysTick_MoreThanOneExpired();

    Assert_Save();
    return 0;
}
