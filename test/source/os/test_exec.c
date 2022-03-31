#include "os/os_p.h"
//#include "cmsis_gcc.h"  // must come after os_p.h
#include "UnitTest.h"

Mock_Vars(3);

#define TEST_UNSET          0x42
#define TEST_CONTEXT_SIZE   42

//Mock_Void(__disable_irq);
//Mock_Void(__enable_irq);
Mock_Void1(os_ContextUse, os_ctx_t *);
Mock_Value(os_actionEntry_t *, os_FifoRemove);
Mock_Void1(os_MemFree, void *);
Mock_Void1(test_Action, os_context_t);

os_actionEntry_t test_actionEntry;
os_ctx_t test_contextEntry;

static void setUp(void)
{
//    Mock_Reset(__disable_irq);
//    Mock_Reset(__enable_irq);
    Mock_Reset(os_ContextUse);
    Mock_Reset(os_FifoRemove);
    Mock_Reset(os_MemFree);
    Mock_Reset(test_Action);

    test_contextEntry.count = 1;
    test_contextEntry.size = TEST_CONTEXT_SIZE;

    test_actionEntry.ctx = &test_contextEntry;
    test_actionEntry.next = NULL;
    test_actionEntry.action = test_Action;
}

static void test_ExecFifoEmpty(void)
{
    setUp();
    Mock_Returns(os_FifoRemove, NULL);

    os_Exec();

    Assert_CalledOnce(os_FifoRemove);

    Assert_NotCalled(test_Action);
}

static void test_ExecFifoNotEmpty(void) {
    setUp();
    Mock_Returns(os_FifoRemove, &test_actionEntry);

    os_Exec();

    Assert_CalledOnce(test_Action);
    Assert_Called1(test_Action, &test_contextEntry.data);

    Assert_Called1(os_ContextUse, &test_contextEntry);
    Assert_Called1(os_MemFree, &test_actionEntry);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_ExecFifoEmpty();
    test_ExecFifoNotEmpty();

    Assert_Save();
    return 0;
}
