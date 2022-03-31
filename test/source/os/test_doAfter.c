#include "os/os_p.h"
//#include "cmsis_gcc.h"  // must come after os_p.h
#include "UnitTest.h"

Mock_Vars(3);

#define TEST_UNSET          0x42
#define TEST_CONTEXT_SIZE   42
#define TEST_TICKS          100

//Mock_Void(__disable_irq);
//Mock_Void(__enable_irq);
Mock_Value1(os_ctx_t *, os_ContextNew, uint32_t);
Mock_Value2(os_context_t, os_Do, os_action_t, uint32_t);
Mock_Value1(void *, os_MemAlloc, uint32_t);
Mock_Void1(os_TimerAdd, os_tqEntry_t *);
Mock_Void1(test_Action, os_context_t);

os_tqEntry_t test_tqEntry;
os_ctx_t test_contextEntry;

static void setUp(void)
{
//    Mock_Reset(__disable_irq);
//    Mock_Reset(__enable_irq);
    Mock_Reset(os_ContextNew);
    Mock_Reset(os_Do);
    Mock_Reset(os_MemAlloc);
    Mock_Reset(os_TimerAdd);
    Mock_Reset(test_Action);

    Mock_Returns(os_Do, test_contextEntry.data);
    Mock_Returns(os_MemAlloc, &test_tqEntry);
    Mock_Returns(os_ContextNew, &test_contextEntry);

    test_tqEntry.action = NULL;
    test_tqEntry.ctx = NULL;
    test_tqEntry.next = NULL;
}

static void test_ZeroTicks(void)
{
    setUp();

    os_context_t result = os_DoAfter(test_Action, TEST_CONTEXT_SIZE, 0);

    Assert_Equals(test_contextEntry.data, result);

    Assert_CalledOnce(os_Do);
    Assert_Called2(os_Do, test_Action, TEST_CONTEXT_SIZE);
}

static void test_AddToQueue(void)
{
    setUp();

    void *result = os_DoAfter(test_Action, TEST_CONTEXT_SIZE, TEST_TICKS);

    Assert_Equals(&(test_contextEntry.data), result);

    Assert_CalledOnce(os_TimerAdd);
    Assert_Called1(os_TimerAdd, &test_tqEntry);
}

static void test_QueuedEntry(void)
{
    setUp();

    os_DoAfter(test_Action, TEST_CONTEXT_SIZE, TEST_TICKS);

    Assert_Called1(os_MemAlloc, sizeof(os_tqEntry_t));

    Assert_CalledOnce(os_ContextNew);
    Assert_Called1(os_ContextNew, TEST_CONTEXT_SIZE);

    Assert_Equals(TEST_TICKS, test_tqEntry.ticks);
    Assert_Equals(&test_Action, test_tqEntry.action);
    Assert_Equals(&test_contextEntry, test_tqEntry.ctx);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_ZeroTicks();
    test_AddToQueue();
    test_QueuedEntry();

    Assert_Save();
    return 0;
}