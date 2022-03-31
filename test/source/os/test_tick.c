#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

Mock_Void1(os_FifoAdd, os_actionEntry_t *);
Mock_Value1(void *, os_MemAlloc, uint32_t);
Mock_Void1(os_MemFree, void *);
Mock_Void1(test_Action, os_context_t);

#define TEST_UNSET          0x42
#define TEST_CONTEXT_SIZE   42
#define TEST_TICKS          2
#define TEST_EXPIRED_TICKS  1
#define TEST_SAME_TIME      0

os_actionEntry_t test_actionEntry;
os_ctx_t test_ctx;
os_tqEntry_t *os_tQueue;
os_tqEntry_t test_tqEntry1;
os_tqEntry_t test_tqEntry2;
os_tqEntry_t test_tqEntry3;

static void setUp(void)
{
    Mock_Reset(os_FifoAdd);
    Mock_Reset(os_MemAlloc);
    Mock_Reset(os_MemFree);
    Mock_Reset(test_Action);

    Mock_Returns(os_MemAlloc, &test_actionEntry);

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

    SysTick_Handler();

    Assert_NotCalled(os_FifoAdd);
    Assert_NotCalled(os_MemFree);
}

static void test_SysTick_TimerNotExpired(void) {
    setUp();

    SysTick_Handler();

    Assert_Equals(TEST_TICKS-1, test_tqEntry1.ticks);
}

static void test_SysTick_TimerExpired_EnqueueAction(void) {
    setUp();
    test_tqEntry1.ticks = TEST_EXPIRED_TICKS;

    SysTick_Handler();

    Assert_CalledOnce(os_MemAlloc);
    Assert_Called1(os_MemAlloc, sizeof(os_actionEntry_t));

    Assert_CalledOnce(os_FifoAdd);
    Assert_Called1(os_FifoAdd, &test_actionEntry);

    Assert_Equals(&test_Action, test_actionEntry.action);
    Assert_Equals(&test_ctx, test_actionEntry.ctx);
}

static void test_SysTick_TimerExpired_UpdateTimerQueue(void) {
    setUp();
    test_tqEntry1.ticks = TEST_EXPIRED_TICKS;

    SysTick_Handler();

    Assert_CalledOnce(os_MemFree);
    Assert_Called1(os_MemFree, &test_tqEntry1);

    Assert_Equals(os_tQueue, &test_tqEntry2);
}

static void test_SysTick_TimerExpired_NoNext(void)
{
    setUp();
    test_tqEntry1.ticks = TEST_EXPIRED_TICKS;
    test_tqEntry1.next = NULL;

    SysTick_Handler();

    Assert_Equals(NULL, os_tQueue);
}

static void test_SysTick_TimerExpired_TickNext(void)
{
    setUp();
    test_tqEntry1.ticks = TEST_EXPIRED_TICKS;

    SysTick_Handler();

    Assert_Equals(&test_tqEntry2, os_tQueue);
}

static void test_SysTick_MoreThanOneExpired(void)
{
    setUp();
    os_tQueue = &test_tqEntry1;
    test_tqEntry1.ticks = TEST_EXPIRED_TICKS;
    test_tqEntry2.ticks = TEST_SAME_TIME;

    SysTick_Handler();

    Assert_CallCount(2, os_MemAlloc);
    Assert_CallCount(2, os_FifoAdd);
    Assert_Equals(&test_tqEntry3, os_tQueue);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_SysTick_Empty();
    test_SysTick_TimerNotExpired();
    test_SysTick_TimerExpired_EnqueueAction();
    test_SysTick_TimerExpired_UpdateTimerQueue();
    test_SysTick_TimerExpired_NoNext();
    test_SysTick_TimerExpired_TickNext();
    test_SysTick_MoreThanOneExpired();

    Assert_Save();
    return 0;
}