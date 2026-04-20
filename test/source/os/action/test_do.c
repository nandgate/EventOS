#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

#define TEST_UNSET          0x42
#define TEST_CONTEXT_SIZE   42

Mock_Void(hal_CriticalBegin);
Mock_Void(hal_CriticalEnd);
Mock_Value1(os_ctx_t *, os_ContextNew, uint32_t);
Mock_Value(os_entry_t *, os_EntryAlloc);
Mock_Void1(os_Fail, os_fail_t);
Mock_Void1(os_FifoAdd, os_entry_t *);
Mock_Void1(test_Action, os_context_t);

os_entry_t test_actionEntry;
os_ctx_t test_contextEntry;

static void setUp(void)
{
    Test_Init();
    Mock_Reset(hal_CriticalBegin);
    Mock_Reset(hal_CriticalEnd);
    Mock_Reset(os_ContextNew);
    Mock_Reset(os_EntryAlloc);
    Mock_Reset(os_Fail);
    Mock_Reset(os_FifoAdd);
    Mock_Reset(test_Action);

    Mock_Returns(os_EntryAlloc, &test_actionEntry);
    Mock_Returns(os_ContextNew, &test_contextEntry);

    test_actionEntry.action = NULL;
    test_actionEntry.ctx = NULL;
    test_actionEntry.key = TEST_UNSET;
    test_actionEntry.next = NULL;
}

static void test_Do_AddToFifo(void)
{
    setUp();

    os_Do(test_Action, TEST_CONTEXT_SIZE);

    Assert_CalledOnce(os_EntryAlloc);
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

static void test_Do_SetsNoKey(void)
{
    setUp();

    os_Do(test_Action, TEST_CONTEXT_SIZE);

    Assert_Equals(OS_NO_KEY, test_actionEntry.key);
}

static void test_Do_NoContext(void)
{
    setUp();
    Mock_Returns(os_ContextNew, NULL);

    void *result = os_Do(test_Action, OS_NO_CONTEXT);

    Assert_IsNull(result);
    Assert_IsNull(test_actionEntry.ctx);
    Assert_CalledOnce(os_FifoAdd);
}

static void test_Do_AllocFails(void)
{
    setUp();
    Mock_Returns(os_EntryAlloc, NULL);

    os_Do(test_Action, TEST_CONTEXT_SIZE);

    Assert_CalledOnce(os_Fail);
    Assert_Called1(os_Fail, OS_FAIL_DO_ALLOCATION);
    Assert_NotCalled(os_FifoAdd);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_Do_AllocFails();
    test_Do_NoContext();
    test_Do_AddToFifo();
    test_Do_Context();
    test_Do_SetsNoKey();

    Assert_Save();
    return 0;
}