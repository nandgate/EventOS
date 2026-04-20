#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(5);

Mock_Void1(os_ContextRelease, os_ctx_t *);
Mock_Void1(os_EntryFree, os_entry_t *);

#define TEST_KEY            0xCAFE
#define TEST_OTHER_KEY      0xBEEF
#define TEST_TICKS_1        100
#define TEST_TICKS_2        20
#define TEST_TICKS_3        50

os_entry_t test_entry1;
os_entry_t test_entry2;
os_entry_t test_entry3;
os_ctx_t test_ctx1;
os_ctx_t test_ctx2;
os_ctx_t test_ctx3;

os_entry_t *os_tQueue;

static void setUp(void) {
    Test_Init();
    Mock_Reset(os_EntryFree);
    Mock_Reset(os_ContextRelease);

    os_tQueue = NULL;

    test_entry1.next = NULL;
    test_entry1.ticks = TEST_TICKS_1;
    test_entry1.key = TEST_KEY;
    test_entry1.action = NULL;
    test_entry1.ctx = &test_ctx1;

    test_entry2.next = NULL;
    test_entry2.ticks = TEST_TICKS_2;
    test_entry2.key = TEST_OTHER_KEY;
    test_entry2.action = NULL;
    test_entry2.ctx = &test_ctx2;

    test_entry3.next = NULL;
    test_entry3.ticks = TEST_TICKS_3;
    test_entry3.key = TEST_KEY;
    test_entry3.action = NULL;
    test_entry3.ctx = &test_ctx3;
}

static void test_RemoveOnlyEntry(void) {
    setUp();
    os_tQueue = &test_entry1;

    os_TimerRemove(TEST_KEY);

    Assert_IsNull(os_tQueue);
}

static void test_RemoveFreesEntryAndContext(void) {
    setUp();
    os_tQueue = &test_entry1;

    os_TimerRemove(TEST_KEY);

    Assert_CalledOnce(os_ContextRelease);
    Assert_Called1(os_ContextRelease, &test_ctx1);
    Assert_CalledOnce(os_EntryFree);
    Assert_Called1(os_EntryFree, &test_entry1);
}

static void test_EmptyQueue(void) {
    setUp();

    os_TimerRemove(TEST_KEY);

    Assert_IsNull(os_tQueue);
    Assert_NotCalled(os_ContextRelease);
}

static void test_NoKeyIsNoop(void) {
    setUp();
    test_entry1.key = OS_NO_KEY;
    os_tQueue = &test_entry1;

    os_TimerRemove(OS_NO_KEY);

    Assert_Equals(&test_entry1, os_tQueue);
    Assert_NotCalled(os_ContextRelease);
}

static void test_NoMatch(void) {
    setUp();
    os_tQueue = &test_entry1;

    os_TimerRemove(0xDEAD);

    Assert_Equals(&test_entry1, os_tQueue);
    Assert_NotCalled(os_ContextRelease);
}

static void test_RemoveMiddleFixesDelta(void) {
    setUp();
    // Queue: entry2(BEEF,20) -> entry1(CAFE,80) -> entry3(unlinked)
    // Delta-encoded: entry2 is 20 absolute, entry1 is +80 (total 100)
    test_entry2.ticks = 20;
    test_entry2.next = &test_entry1;
    test_entry1.ticks = 80;
    test_entry1.next = &test_entry3;
    test_entry3.ticks = 50;
    test_entry3.next = NULL;
    test_entry3.key = TEST_OTHER_KEY;
    os_tQueue = &test_entry2;

    os_TimerRemove(TEST_KEY);

    // entry1 removed; entry3's ticks must absorb entry1's delta
    Assert_Equals(&test_entry2, os_tQueue);
    Assert_Equals(&test_entry3, test_entry2.next);
    Assert_Equals(130, test_entry3.ticks);
    Assert_IsNull(test_entry3.next);
}

static void test_RemoveHeadFixesDelta(void) {
    setUp();
    // Queue: entry1(CAFE,100) -> entry2(BEEF,20)
    test_entry1.next = &test_entry2;
    os_tQueue = &test_entry1;

    os_TimerRemove(TEST_KEY);

    Assert_Equals(&test_entry2, os_tQueue);
    Assert_Equals(120, test_entry2.ticks);
    Assert_IsNull(test_entry2.next);
}

static void test_RemoveAllMatching(void) {
    setUp();
    // Queue: entry1(CAFE,100) -> entry3(CAFE,50) -> entry2(BEEF,20)
    test_entry1.next = &test_entry3;
    test_entry3.key = TEST_KEY;
    test_entry3.next = &test_entry2;
    os_tQueue = &test_entry1;

    os_TimerRemove(TEST_KEY);

    Assert_Equals(&test_entry2, os_tQueue);
    Assert_Equals(170, test_entry2.ticks);
}

static void test_RemoveTail(void) {
    setUp();
    // Queue: entry2(BEEF,20) -> entry1(CAFE,80)
    test_entry2.next = &test_entry1;
    test_entry1.ticks = 80;
    os_tQueue = &test_entry2;

    os_TimerRemove(TEST_KEY);

    Assert_Equals(&test_entry2, os_tQueue);
    Assert_IsNull(test_entry2.next);
    Assert_Equals(20, test_entry2.ticks);
}

int main(int argc, char **argv) {
    Assert_Init();

    test_EmptyQueue();
    test_NoKeyIsNoop();
    test_NoMatch();
    test_RemoveOnlyEntry();
    test_RemoveFreesEntryAndContext();
    test_RemoveHeadFixesDelta();
    test_RemoveMiddleFixesDelta();
    test_RemoveTail();
    test_RemoveAllMatching();

    Assert_Save();
    return 0;
}
