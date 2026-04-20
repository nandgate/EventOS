#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

#define TEST_UNSET          0x42
#define TEST_CONTEXT_SIZE   42

Mock_Void(hal_CriticalBegin);
Mock_Void(hal_CriticalEnd);
Mock_Value1(os_ctx_t *, os_ContextAcquire, os_context_t);
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
    Mock_Reset(os_ContextAcquire);
    Mock_Reset(os_EntryAlloc);
    Mock_Reset(os_Fail);
    Mock_Reset(os_FifoAdd);
    Mock_Reset(test_Action);

    Mock_Returns(os_EntryAlloc, &test_actionEntry);
    Mock_Returns(os_ContextAcquire, &test_contextEntry);

    test_actionEntry.action = NULL;
    test_actionEntry.ctx = NULL;
    test_actionEntry.key = TEST_UNSET;
    test_actionEntry.next = NULL;

    test_contextEntry.count = 1;
}

static void test_DoWith_AddToFifo(void)
{
    setUp();

    os_DoWith(test_Action, &test_contextEntry.data);

    Assert_CalledOnce(os_EntryAlloc);
    Assert_Called1(os_FifoAdd, &test_actionEntry);
    Assert_Equals(test_Action, test_actionEntry.action);
}

static void test_DoWith_Context(void)
{
    setUp();

    os_DoWith(test_Action, &test_contextEntry.data);

    Assert_Equals(&test_contextEntry, test_actionEntry.ctx);

    Assert_CalledOnce(os_ContextAcquire);
    Assert_Called1(os_ContextAcquire, &test_contextEntry.data);
}

static void test_DoWith_SetsNoKey(void)
{
    setUp();

    os_DoWith(test_Action, &test_contextEntry.data);

    Assert_Equals(OS_NO_KEY, test_actionEntry.key);
}

static void test_DoWith_AllocFails(void)
{
    setUp();
    Mock_Returns(os_EntryAlloc, NULL);

    os_DoWith(test_Action, &test_contextEntry.data);

    Assert_CalledOnce(os_Fail);
    Assert_Called1(os_Fail, OS_FAIL_DO_WITH_ALLOCATION);
    Assert_NotCalled(os_FifoAdd);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_DoWith_AllocFails();
    test_DoWith_AddToFifo();
    test_DoWith_Context();
    test_DoWith_SetsNoKey();
    Assert_Save();
    return 0;
}