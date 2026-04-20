#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

#define TEST_UNSET          0x42
#define TEST_CONTEXT_SIZE   42
#define TEST_KEY            0xCAFE
#define TEST_TICKS          100

Mock_Void1(os_CancelPending, uint32_t);
Mock_Void(hal_CriticalBegin);
Mock_Void(hal_CriticalEnd);
Mock_Value1(os_ctx_t *, os_ContextNew, uint32_t);
Mock_Value(os_entry_t *, os_EntryAlloc);
Mock_Void1(os_Fail, os_fail_t);
Mock_Void1(os_FifoAdd, os_entry_t *);
Mock_Void1(os_TimerAdd, os_entry_t *);
Mock_Void1(test_Action, os_context_t);

os_entry_t test_tqEntry;
os_ctx_t test_contextEntry;

static void setUp(void)
{
    Test_Init();
    Mock_Reset(os_CancelPending);
    Mock_Reset(hal_CriticalBegin);
    Mock_Reset(hal_CriticalEnd);
    Mock_Reset(os_ContextNew);
    Mock_Reset(os_EntryAlloc);
    Mock_Reset(os_Fail);
    Mock_Reset(os_FifoAdd);
    Mock_Reset(os_TimerAdd);
    Mock_Reset(test_Action);

    Mock_Returns(os_EntryAlloc, &test_tqEntry);
    Mock_Returns(os_ContextNew, &test_contextEntry);

    test_tqEntry.action = NULL;
    test_tqEntry.ctx = NULL;
    test_tqEntry.next = NULL;
    test_tqEntry.key = TEST_UNSET;
    test_tqEntry.ticks = TEST_UNSET;
}

static void test_ZeroTicks_GoesToFifo(void)
{
    setUp();

    os_context_t result = os_DoAfter(test_Action, TEST_CONTEXT_SIZE, OS_NO_KEY, 0);

    Assert_Equals(&(test_contextEntry.data), result);
    Assert_CalledOnce(os_FifoAdd);
    Assert_Called1(os_FifoAdd, &test_tqEntry);
    Assert_NotCalled(os_TimerAdd);
}

static void test_ZeroTicks_PreservesKey(void)
{
    setUp();

    os_DoAfter(test_Action, TEST_CONTEXT_SIZE, TEST_KEY, 0);

    Assert_Equals(TEST_KEY, test_tqEntry.key);
}

static void test_NonZeroTicks_GoesToTimer(void)
{
    setUp();

    void *result = os_DoAfter(test_Action, TEST_CONTEXT_SIZE, OS_NO_KEY, TEST_TICKS);

    Assert_Equals(&(test_contextEntry.data), result);
    Assert_CalledOnce(os_TimerAdd);
    Assert_Called1(os_TimerAdd, &test_tqEntry);
    Assert_NotCalled(os_FifoAdd);
}

static void test_QueuedEntry(void)
{
    setUp();

    os_DoAfter(test_Action, TEST_CONTEXT_SIZE, OS_NO_KEY, TEST_TICKS);

    Assert_CalledOnce(os_EntryAlloc);

    Assert_CalledOnce(os_ContextNew);
    Assert_Called1(os_ContextNew, TEST_CONTEXT_SIZE);

    Assert_Equals(TEST_TICKS, test_tqEntry.ticks);
    Assert_Equals(&test_Action, test_tqEntry.action);
    Assert_Equals(&test_contextEntry, test_tqEntry.ctx);
}

static void test_QueuedEntryStoresKey(void)
{
    setUp();

    os_DoAfter(test_Action, TEST_CONTEXT_SIZE, TEST_KEY, TEST_TICKS);

    Assert_Equals(TEST_KEY, test_tqEntry.key);
}

static void test_WithKey_CancelsExisting(void)
{
    setUp();

    os_DoAfter(test_Action, TEST_CONTEXT_SIZE, TEST_KEY, TEST_TICKS);

    Assert_CalledOnce(os_CancelPending);
    Assert_Called1(os_CancelPending, TEST_KEY);
}

static void test_NoKey_SkipsCancel(void)
{
    setUp();

    os_DoAfter(test_Action, TEST_CONTEXT_SIZE, OS_NO_KEY, TEST_TICKS);

    Assert_NotCalled(os_CancelPending);
}

static void test_NoContext(void)
{
    setUp();
    Mock_Returns(os_ContextNew, NULL);

    void *result = os_DoAfter(test_Action, OS_NO_CONTEXT, OS_NO_KEY, TEST_TICKS);

    Assert_IsNull(result);
    Assert_IsNull(test_tqEntry.ctx);
    Assert_CalledOnce(os_TimerAdd);
}

static void test_AllocFails(void)
{
    setUp();
    Mock_Returns(os_EntryAlloc, NULL);

    os_DoAfter(test_Action, TEST_CONTEXT_SIZE, TEST_KEY, TEST_TICKS);

    Assert_CalledOnce(os_Fail);
    Assert_Called1(os_Fail, OS_FAIL_DO_AFTER_ALLOCATION);
    Assert_NotCalled(os_TimerAdd);
    Assert_NotCalled(os_FifoAdd);
    Assert_NotCalled(os_CancelPending);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_AllocFails();
    test_NoContext();
    test_ZeroTicks_GoesToFifo();
    test_ZeroTicks_PreservesKey();
    test_NonZeroTicks_GoesToTimer();
    test_QueuedEntry();
    test_QueuedEntryStoresKey();
    test_WithKey_CancelsExisting();
    test_NoKey_SkipsCancel();

    Assert_Save();
    return 0;
}
