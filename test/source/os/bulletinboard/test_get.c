#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

Mock_Void(hal_CriticalBegin);
Mock_Void(hal_CriticalEnd);
Mock_Void1(os_EntryFree, os_entry_t *);

#define TEST_KEY        0xCAFE
#define TEST_OTHER_KEY  0xBEEF

os_entry_t test_entry1;
os_entry_t test_entry2;
os_entry_t test_entry3;
os_ctx_t test_ctx1;
os_ctx_t test_ctx2;
os_ctx_t test_ctx3;

os_actionFifo_t os_actionFifo;
os_entry_t *os_tQueue;
os_ctx_t *os_heldCtx;

static void setUp(void) {
    Test_Init();
    Mock_Reset(hal_CriticalBegin);
    Mock_Reset(hal_CriticalEnd);
    Mock_Reset(os_EntryFree);

    os_tQueue = NULL;
    os_actionFifo.os_fifoHead = NULL;
    os_actionFifo.os_fifoTail = NULL;
    os_heldCtx = NULL;

    test_entry1.next = NULL;
    test_entry1.key = TEST_KEY;
    test_entry1.ticks = 100;
    test_entry1.ctx = &test_ctx1;
    test_entry1.action = NULL;

    test_entry2.next = NULL;
    test_entry2.key = TEST_OTHER_KEY;
    test_entry2.ticks = 50;
    test_entry2.ctx = &test_ctx2;
    test_entry2.action = NULL;

    test_entry3.next = NULL;
    test_entry3.key = TEST_KEY;
    test_entry3.ticks = 30;
    test_entry3.ctx = &test_ctx3;
    test_entry3.action = NULL;
}

static void test_EmptyQueues(void) {
    setUp();

    os_context_t result = os_Get(TEST_KEY);

    Assert_IsNull(result);
    Assert_NotCalled(os_EntryFree);
}

static void test_NoMatch(void) {
    setUp();
    os_tQueue = &test_entry2;
    os_actionFifo.os_fifoHead = &test_entry2;
    os_actionFifo.os_fifoTail = &test_entry2;

    os_context_t result = os_Get(TEST_KEY);

    Assert_IsNull(result);
    Assert_NotCalled(os_EntryFree);
}

static void test_OSNoKey_IsNoop(void) {
    setUp();
    test_entry1.key = OS_NO_KEY;
    os_tQueue = &test_entry1;

    os_context_t result = os_Get(OS_NO_KEY);

    Assert_IsNull(result);
    Assert_Equals(&test_entry1, os_tQueue);
    Assert_NotCalled(os_EntryFree);
}

static void test_FoundInTimerQueue_Head(void) {
    setUp();
    os_tQueue = &test_entry1;

    os_context_t result = os_Get(TEST_KEY);

    Assert_Equals(test_ctx1.data, result);
    Assert_IsNull(os_tQueue);
    Assert_CalledOnce(os_EntryFree);
    Assert_Called1(os_EntryFree, &test_entry1);
}

static void test_FoundInTimerQueue_Middle_AdjustsDelta(void) {
    setUp();
    // Queue: entry2(OTHER,50) -> entry1(CAFE,100) -> entry3(CAFE,30)
    // We search for TEST_KEY, should find entry1 first; entry3's delta absorbs entry1's.
    test_entry2.next = &test_entry1;
    test_entry1.next = &test_entry3;
    os_tQueue = &test_entry2;

    os_context_t result = os_Get(TEST_KEY);

    Assert_Equals(test_ctx1.data, result);
    Assert_Equals(&test_entry2, os_tQueue);
    Assert_Equals(&test_entry3, test_entry2.next);
    Assert_Equals(130, test_entry3.ticks);
    Assert_CalledOnce(os_EntryFree);
    Assert_Called1(os_EntryFree, &test_entry1);
}

static void test_FoundInFifo_Head(void) {
    setUp();
    os_actionFifo.os_fifoHead = &test_entry1;
    os_actionFifo.os_fifoTail = &test_entry1;

    os_context_t result = os_Get(TEST_KEY);

    Assert_Equals(test_ctx1.data, result);
    Assert_IsNull(os_actionFifo.os_fifoHead);
    Assert_IsNull(os_actionFifo.os_fifoTail);
    Assert_CalledOnce(os_EntryFree);
    Assert_Called1(os_EntryFree, &test_entry1);
}

static void test_FoundInFifo_Tail_UpdatesTail(void) {
    setUp();
    // FIFO: entry2(OTHER) -> entry1(CAFE)
    test_entry2.next = &test_entry1;
    os_actionFifo.os_fifoHead = &test_entry2;
    os_actionFifo.os_fifoTail = &test_entry1;

    os_context_t result = os_Get(TEST_KEY);

    Assert_Equals(test_ctx1.data, result);
    Assert_Equals(&test_entry2, os_actionFifo.os_fifoHead);
    Assert_Equals(&test_entry2, os_actionFifo.os_fifoTail);
    Assert_IsNull(test_entry2.next);
}

static void test_SetsHeldCtx(void) {
    setUp();
    os_tQueue = &test_entry1;

    os_Get(TEST_KEY);

    Assert_Equals(&test_ctx1, os_heldCtx);
    Assert_IsNull(test_ctx1.next);
}

static void test_PrependsToHeldList(void) {
    setUp();
    os_tQueue = &test_entry1;
    os_heldCtx = &test_ctx2;

    os_Get(TEST_KEY);

    // New ctx is prepended; prior held ctx is preserved, not released.
    Assert_Equals(&test_ctx1, os_heldCtx);
    Assert_Equals(&test_ctx2, test_ctx1.next);
}

static void test_WrapsListWalkInCriticalSection(void) {
    setUp();
    os_tQueue = &test_entry1;

    os_Get(TEST_KEY);

    Assert_CalledOnce(hal_CriticalBegin);
    Assert_CalledOnce(hal_CriticalEnd);
}

int main(int argc, char **argv) {
    Assert_Init();

    test_EmptyQueues();
    test_NoMatch();
    test_OSNoKey_IsNoop();
    test_FoundInTimerQueue_Head();
    test_FoundInTimerQueue_Middle_AdjustsDelta();
    test_FoundInFifo_Head();
    test_FoundInFifo_Tail_UpdatesTail();
    test_SetsHeldCtx();
    test_PrependsToHeldList();
    test_WrapsListWalkInCriticalSection();

    Assert_Save();
    return 0;
}
