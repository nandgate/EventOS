#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(5);

Mock_Void1(os_ContextRelease, os_ctx_t *);
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

static void setUp(void) {
    Test_Init();
    Mock_Reset(os_EntryFree);
    Mock_Reset(os_ContextRelease);

    os_actionFifo.os_fifoHead = NULL;
    os_actionFifo.os_fifoTail = NULL;

    test_entry1.next = NULL;
    test_entry1.key = TEST_KEY;
    test_entry1.ctx = &test_ctx1;
    test_entry1.action = NULL;

    test_entry2.next = NULL;
    test_entry2.key = TEST_OTHER_KEY;
    test_entry2.ctx = &test_ctx2;
    test_entry2.action = NULL;

    test_entry3.next = NULL;
    test_entry3.key = TEST_KEY;
    test_entry3.ctx = &test_ctx3;
    test_entry3.action = NULL;
}

static void test_EmptyFifo(void) {
    setUp();

    os_FifoCancel(TEST_KEY);

    Assert_IsNull(os_actionFifo.os_fifoHead);
    Assert_IsNull(os_actionFifo.os_fifoTail);
    Assert_NotCalled(os_ContextRelease);
}

static void test_NoKeyIsNoop(void) {
    setUp();
    test_entry1.key = OS_NO_KEY;
    os_actionFifo.os_fifoHead = &test_entry1;
    os_actionFifo.os_fifoTail = &test_entry1;

    os_FifoCancel(OS_NO_KEY);

    Assert_Equals(&test_entry1, os_actionFifo.os_fifoHead);
    Assert_NotCalled(os_ContextRelease);
}

static void test_RemoveOnlyEntry(void) {
    setUp();
    os_actionFifo.os_fifoHead = &test_entry1;
    os_actionFifo.os_fifoTail = &test_entry1;

    os_FifoCancel(TEST_KEY);

    Assert_IsNull(os_actionFifo.os_fifoHead);
    Assert_IsNull(os_actionFifo.os_fifoTail);
}

static void test_RemoveFreesEntryAndContext(void) {
    setUp();
    os_actionFifo.os_fifoHead = &test_entry1;
    os_actionFifo.os_fifoTail = &test_entry1;

    os_FifoCancel(TEST_KEY);

    Assert_CalledOnce(os_ContextRelease);
    Assert_Called1(os_ContextRelease, &test_ctx1);
    Assert_CalledOnce(os_EntryFree);
    Assert_Called1(os_EntryFree, &test_entry1);
}

static void test_RemoveHead(void) {
    setUp();
    // FIFO: entry1(CAFE) -> entry2(BEEF)
    test_entry1.next = &test_entry2;
    os_actionFifo.os_fifoHead = &test_entry1;
    os_actionFifo.os_fifoTail = &test_entry2;

    os_FifoCancel(TEST_KEY);

    Assert_Equals(&test_entry2, os_actionFifo.os_fifoHead);
    Assert_Equals(&test_entry2, os_actionFifo.os_fifoTail);
}

static void test_RemoveMiddle(void) {
    setUp();
    // FIFO: entry2(BEEF) -> entry1(CAFE) -> entry3(CAFE)
    test_entry2.next = &test_entry1;
    test_entry1.next = &test_entry3;
    test_entry3.key = TEST_KEY;
    os_actionFifo.os_fifoHead = &test_entry2;
    os_actionFifo.os_fifoTail = &test_entry3;

    os_FifoCancel(TEST_KEY);

    Assert_Equals(&test_entry2, os_actionFifo.os_fifoHead);
    Assert_Equals(&test_entry2, os_actionFifo.os_fifoTail);
    Assert_IsNull(test_entry2.next);
}

static void test_RemoveTail(void) {
    setUp();
    // FIFO: entry2(BEEF) -> entry1(CAFE)
    test_entry2.next = &test_entry1;
    os_actionFifo.os_fifoHead = &test_entry2;
    os_actionFifo.os_fifoTail = &test_entry1;

    os_FifoCancel(TEST_KEY);

    Assert_Equals(&test_entry2, os_actionFifo.os_fifoHead);
    Assert_Equals(&test_entry2, os_actionFifo.os_fifoTail);
    Assert_IsNull(test_entry2.next);
}

static void test_NoMatch(void) {
    setUp();
    os_actionFifo.os_fifoHead = &test_entry2;
    os_actionFifo.os_fifoTail = &test_entry2;

    os_FifoCancel(TEST_KEY);

    Assert_Equals(&test_entry2, os_actionFifo.os_fifoHead);
    Assert_NotCalled(os_ContextRelease);
}

int main(int argc, char **argv) {
    Assert_Init();

    test_EmptyFifo();
    test_NoKeyIsNoop();
    test_NoMatch();
    test_RemoveOnlyEntry();
    test_RemoveFreesEntryAndContext();
    test_RemoveHead();
    test_RemoveMiddle();
    test_RemoveTail();

    Assert_Save();
    return 0;
}
