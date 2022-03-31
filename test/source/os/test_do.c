//#include "cmsis_gcc.h"
#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

#define TEST_UNSET          0x42
#define TEST_CONTEXT_SIZE   42

//Mock_Void(__disable_irq);
//Mock_Void(__enable_irq);
Mock_Value1(os_ctx_t *, os_ContextNew, uint32_t);
Mock_Void1(os_FifoAdd, os_actionEntry_t *);
Mock_Value1(void *, os_MemAlloc, uint32_t);
Mock_Void1(test_Action, os_context_t);

os_actionEntry_t test_actionEntry;
os_ctx_t test_contextEntry;

static void setUp(void)
{
//    Mock_Reset(__disable_irq);
//    Mock_Reset(__enable_irq);
    Mock_Reset(os_ContextNew);
    Mock_Reset(os_FifoAdd);
    Mock_Reset(os_MemAlloc);
    Mock_Reset(test_Action);

    Mock_Returns(os_MemAlloc, &test_actionEntry);
    Mock_Returns(os_ContextNew, &test_contextEntry);

    test_actionEntry.action = NULL;
    test_actionEntry.ctx = NULL;
    test_actionEntry.next = NULL;
}

static void test_Do_AddToFifo(void)
{
    setUp();

    os_Do(test_Action, TEST_CONTEXT_SIZE);

    Assert_Called1(os_MemAlloc, sizeof(os_actionEntry_t));
    Assert_Called1(os_FifoAdd, &test_actionEntry);
    Assert_Equals(test_Action, test_actionEntry.action);
}

static void test_Do_Context(void)
{
    setUp();

    void *result = os_Do(test_Action, TEST_CONTEXT_SIZE);

    Assert_Equals(&test_contextEntry.data, result);

    Assert_Called1(os_ContextNew, TEST_CONTEXT_SIZE);
    Assert_Equals(&test_contextEntry, test_actionEntry.ctx);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_Do_AddToFifo();
    test_Do_Context();

    Assert_Save();
    return 0;
}