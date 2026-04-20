#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

#define TEST_UNSET          0x42
#define TEST_KEY            0xCAFE
#define TEST_CONTEXT_SIZE   16
#define TEST_TICKS          500

Mock_Void1(os_CancelPending, uint32_t);
Mock_Void(hal_CriticalBegin);
Mock_Void(hal_CriticalEnd);
Mock_Value1(os_ctx_t *, os_ContextNew, uint32_t);
Mock_Value(os_entry_t *, os_EntryAlloc);
Mock_Void1(os_Fail, os_fail_t);
Mock_Void1(os_FifoAdd, os_entry_t *);
Mock_Void1(os_TimerAdd, os_entry_t *);
Mock_Void1(test_TimeoutAction, os_context_t);

os_entry_t test_entry;
os_ctx_t test_ctx;

static void setUp(void) {
    Test_Init();
    Mock_Reset(os_CancelPending);
    Mock_Reset(hal_CriticalBegin);
    Mock_Reset(hal_CriticalEnd);
    Mock_Reset(os_ContextNew);
    Mock_Reset(os_EntryAlloc);
    Mock_Reset(os_Fail);
    Mock_Reset(os_FifoAdd);
    Mock_Reset(os_TimerAdd);
    Mock_Reset(test_TimeoutAction);

    Mock_Returns(os_EntryAlloc, &test_entry);
    Mock_Returns(os_ContextNew, &test_ctx);

    test_entry.next = NULL;
    test_entry.key = TEST_UNSET;
    test_entry.ticks = TEST_UNSET;
    test_entry.action = NULL;
    test_entry.ctx = NULL;
}

static void test_AllocatesContext(void) {
    setUp();

    os_context_t result = os_Put(TEST_KEY, TEST_CONTEXT_SIZE, test_TimeoutAction, TEST_TICKS);

    Assert_CalledOnce(os_ContextNew);
    Assert_Called1(os_ContextNew, TEST_CONTEXT_SIZE);
    Assert_Equals(&test_ctx.data, result);
}

static void test_PopulatesEntry(void) {
    setUp();

    os_Put(TEST_KEY, TEST_CONTEXT_SIZE, test_TimeoutAction, TEST_TICKS);

    Assert_Equals(TEST_KEY, test_entry.key);
    Assert_Equals(TEST_TICKS, test_entry.ticks);
    Assert_Equals(test_TimeoutAction, test_entry.action);
    Assert_Equals(&test_ctx, test_entry.ctx);
}

static void test_NonZeroTicks_GoesToTimer(void) {
    setUp();

    os_Put(TEST_KEY, TEST_CONTEXT_SIZE, test_TimeoutAction, TEST_TICKS);

    Assert_CalledOnce(os_TimerAdd);
    Assert_Called1(os_TimerAdd, &test_entry);
    Assert_NotCalled(os_FifoAdd);
}

static void test_ZeroTicks_GoesToFifo(void) {
    setUp();

    os_Put(TEST_KEY, TEST_CONTEXT_SIZE, test_TimeoutAction, 0);

    Assert_CalledOnce(os_FifoAdd);
    Assert_Called1(os_FifoAdd, &test_entry);
    Assert_NotCalled(os_TimerAdd);
}

static void test_ZeroTicks_EntryKeyIsNoKey(void) {
    setUp();

    os_Put(TEST_KEY, TEST_CONTEXT_SIZE, test_TimeoutAction, 0);

    Assert_Equals(OS_NO_KEY, test_entry.key);
}

static void test_WithKey_CancelsExisting(void) {
    setUp();

    os_Put(TEST_KEY, TEST_CONTEXT_SIZE, test_TimeoutAction, TEST_TICKS);

    Assert_CalledOnce(os_CancelPending);
    Assert_Called1(os_CancelPending, TEST_KEY);
}

static void test_NoKey_SkipsCancel(void) {
    setUp();

    os_Put(OS_NO_KEY, TEST_CONTEXT_SIZE, test_TimeoutAction, TEST_TICKS);

    Assert_NotCalled(os_CancelPending);
}

static void test_AllocFails(void) {
    setUp();
    Mock_Returns(os_EntryAlloc, NULL);

    os_context_t result = os_Put(TEST_KEY, TEST_CONTEXT_SIZE, test_TimeoutAction, TEST_TICKS);

    Assert_IsNull(result);
    Assert_CalledOnce(os_Fail);
    Assert_Called1(os_Fail, OS_FAIL_PUT_ALLOCATION);
    Assert_NotCalled(os_TimerAdd);
    Assert_NotCalled(os_FifoAdd);
    Assert_NotCalled(os_CancelPending);
}

static void test_WrapsInCriticalSection(void) {
    setUp();

    os_Put(TEST_KEY, TEST_CONTEXT_SIZE, test_TimeoutAction, TEST_TICKS);

    Assert_CalledOnce(hal_CriticalBegin);
    Assert_CalledOnce(hal_CriticalEnd);
}

int main(int argc, char **argv) {
    Assert_Init();

    test_AllocatesContext();
    test_PopulatesEntry();
    test_NonZeroTicks_GoesToTimer();
    test_ZeroTicks_GoesToFifo();
    test_ZeroTicks_EntryKeyIsNoKey();
    test_WithKey_CancelsExisting();
    test_NoKey_SkipsCancel();
    test_AllocFails();
    test_WrapsInCriticalSection();

    Assert_Save();
    return 0;
}
