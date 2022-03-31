#include "os/os_p.h"
//#include "cmsis_gcc.h"  // must come after os_p.h
#include "UnitTest.h"

Mock_Vars(3);

#define TEST_UNSET          0x42
#define TEST_TICKS          100

//Mock_Void(__disable_irq);
//Mock_Void(__enable_irq);
Mock_Value1(os_ctx_t *, os_ContextReuse, os_context_t);
Mock_Void2(os_DoWith, os_action_t, os_context_t);
Mock_Value1(void *, os_MemAlloc, uint32_t);
Mock_Void1(os_TimerAdd, os_tqEntry_t *);
Mock_Void1(test_Action, os_context_t);

os_tqEntry_t test_tqEntry;
os_ctx_t test_ctx;

static void setUp(void)
{
//    Mock_Reset(__disable_irq);
//    Mock_Reset(__enable_irq);
    Mock_Reset(os_ContextReuse);
    Mock_Reset(os_DoWith);
    Mock_Reset(os_MemAlloc);
    Mock_Reset(os_TimerAdd);
    Mock_Reset(test_Action);

    Mock_Returns(os_ContextReuse, &test_ctx);
    Mock_Returns(os_MemAlloc, &test_tqEntry);

    test_tqEntry.action = NULL;
    test_tqEntry.ctx = NULL;
    test_tqEntry.next = NULL;
}

static void test_ZeroTicks(void)
{
    setUp();

    os_DoAfterWith(test_Action, test_ctx.data, 0);

    Assert_CalledOnce(os_DoWith);
    Assert_Called2(os_DoWith, test_Action, test_ctx.data);
}

static void test_AddToQueue(void)
{
    setUp();

    os_DoAfterWith(test_Action,  test_ctx.data, TEST_TICKS);

    Assert_CalledOnce(os_ContextReuse);
    Assert_Called1(os_ContextReuse, &(test_ctx.data));

    Assert_CalledOnce(os_TimerAdd);
    Assert_Called1(os_TimerAdd, &test_tqEntry);
}

static void test_QueuedEntry(void)
{
    setUp();

    os_DoAfterWith(test_Action,  test_ctx.data, TEST_TICKS);

    Assert_Called1(os_MemAlloc, sizeof(os_tqEntry_t));

    Assert_Equals(TEST_TICKS, test_tqEntry.ticks);
    Assert_Equals(&test_Action, test_tqEntry.action);
    Assert_Equals(&test_ctx, test_tqEntry.ctx);
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