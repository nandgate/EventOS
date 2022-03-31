#include "os/os_p.h"
//#include "cmsis_gcc.h"  // must come after os_p.h
#include "UnitTest.h"

Mock_Vars(3);

#define TEST_UNSET          0x42
#define TEST_CONTEXT_SIZE   42

//Mock_Void(__disable_irq);
//Mock_Void(__enable_irq);
Mock_Value1(os_ctx_t *, os_ContextReuse, os_context_t);
Mock_Void1(os_FifoAdd, os_actionEntry_t *);
Mock_Value1(void *, os_MemAlloc, uint32_t);
Mock_Void1(test_Action, os_context_t);

os_actionEntry_t test_actionEntry;
os_ctx_t test_contextEntry;

static void setUp(void)
{
//    Mock_Reset(__disable_irq);
//    Mock_Reset(__enable_irq);
    Mock_Reset(os_ContextReuse);
    Mock_Reset(os_FifoAdd);
    Mock_Reset(os_MemAlloc);
    Mock_Reset(test_Action);

    Mock_Returns(os_MemAlloc, &test_actionEntry);
    Mock_Returns(os_ContextReuse, &test_contextEntry);

    test_actionEntry.action = NULL;
    test_actionEntry.ctx = NULL;
    test_actionEntry.next = NULL;

    test_contextEntry.count = 1;
}

static void test_DoWith_AddToFifo(void)
{
    setUp();

    os_DoWith(test_Action, &test_contextEntry.data);

    Assert_Called1(os_MemAlloc, sizeof(os_actionEntry_t));
    Assert_Called1(os_FifoAdd, &test_actionEntry);
    Assert_Equals(test_Action, test_actionEntry.action);
}

static void test_DoWith_Context(void)
{
    setUp();

    os_DoWith(test_Action, &test_contextEntry.data);

    Assert_Equals(&test_contextEntry, test_actionEntry.ctx);

    Assert_CalledOnce(os_ContextReuse);
    Assert_Called1(os_ContextReuse, &test_contextEntry.data);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_DoWith_AddToFifo();
    test_DoWith_Context();
    Assert_Save();
    return 0;
}