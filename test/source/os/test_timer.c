#include "os/os_p.h"
#include "UnitTest.h"

// Note:
// There are MANY test cases in this file. Many of them are redundant.
// Getting the os_TimerAdd function working correctly took a lot of effort
// and more than a few coding restarts to finally get it working correctly.
// Due to the difficulties encountered I abandoned the usual "test just the
// use cases" approach and switched to the "test all the permutations"
// approach instead. Thus, lots of test cases.

Mock_Vars(3);

#define TEST_UNSET          0x42
#define TEST_TICKS          4242
arm_SCS_t test_system;

os_tqEntry_t *os_tQueue;
os_tqEntry_t test_tqEntryA;
os_tqEntry_t test_tqEntryB;
os_tqEntry_t test_tqEntryC;
os_tqEntry_t test_tqEntryD;
os_tqEntry_t *test_tqEntryPtr;

static void setUp(char *name)
{
    Assert_Note(name);
    test_system.systick.CTRL = TEST_UNSET;
    test_system.systick.LOAD = TEST_UNSET;
    test_system.systick.VAL = TEST_UNSET;
    test_system.scb.SHPR3 = TEST_UNSET;

    os_tQueue = NULL;
    test_tqEntryPtr = NULL;
}

// Helper function to add an entry to the timer queue
static void test_AddEntry(os_tqEntry_t *e, uint32_t t) {
    if (test_tqEntryPtr == NULL) {
        os_tQueue = e;
    }
    else {
        test_tqEntryPtr->next = e;
    }
    test_tqEntryPtr = e;
    e->next = NULL;
    e->ticks = t;
}

// Helper function the call the OS_TimerAdd with an timer queue entry
static void test_Add(os_tqEntry_t *entry, uint32_t t) {
    entry->ticks = t;
    entry->next = NULL;

    os_TimerAdd(entry);

    test_tqEntryPtr = os_tQueue;    // prep for AssertEntry
}

// Helper function to assert on the state of a timer queue entry in the list, also iterates the list.
static void test_AssertEntry(os_tqEntry_t *entry, uint32_t t) {
    Assert_True(test_tqEntryPtr != NULL);
    Assert_Equals(entry, test_tqEntryPtr);
    Assert_Equals(t, entry->ticks);
    test_tqEntryPtr = entry->next;
}

// Helper function to assert that we have iterated to the end of the list.
void test_AssertEnd(void) {
    Assert_Equals(NULL, test_tqEntryPtr);
}

static void test_Init(void)
{
    setUp("test_Init");
    os_tQueue =  (os_tqEntry_t *) TEST_UNSET;

    os_TimerInit(&test_system, TEST_TICKS);

    Assert_Equals(NULL, os_tQueue);

    Assert_Equals(TEST_TICKS, test_system.systick.LOAD);
    Assert_Equals(0xFF000000 | TEST_UNSET, test_system.scb.SHPR3);
    Assert_Equals(0x00, test_system.systick.VAL);
    Assert_Equals(0x07, test_system.systick.CTRL);
}

static void test_Empty(void) {
    setUp("test_Empty");

    test_Add(&test_tqEntryD, 1);

    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEnd();
}

static void test_One_InsertBefore(void) {
    setUp("test_One_InsertBefore");
    test_AddEntry(&test_tqEntryA, 2);

    test_Add(&test_tqEntryD, 1);

    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEntry(&test_tqEntryA, 1);
    test_AssertEnd();
}

static void test_One_InsertMatching(void) {
    setUp("test_One_InsertMatching");
    test_AddEntry(&test_tqEntryA, 2);

    test_Add(&test_tqEntryD, 2);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryD, 0);
    test_AssertEnd();
}

static void test_One_InsertAfter(void) {
    setUp("test_One_InsertAfter");
    test_AddEntry(&test_tqEntryA, 2);

    test_Add(&test_tqEntryD, 3);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEnd();
}

static void test_TwoSame_InsertBefore(void) {
    setUp("test_TwoSame_InsertBefore");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 0);

    test_Add(&test_tqEntryD, 1);

    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEntry(&test_tqEntryA, 1);
    test_AssertEntry(&test_tqEntryB, 0);
    test_AssertEnd();
}

static void test_TwoSame_InsertSame(void) {
    setUp("test_TwoSame_InsertSame");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 0);

    test_Add(&test_tqEntryD, 2);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 0);
    test_AssertEntry(&test_tqEntryD, 0);
    test_AssertEnd();
}

static void test_TwoSame_InsertAfter(void) {
    setUp("test_TwoSame_InsertAfter");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 0);

    test_Add(&test_tqEntryD, 3);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 0);
    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEnd();
}

static void test_TwoWithGap_InsertBefore(void) {
    setUp("test_TwoWithGap_InsertBefore");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 2);

    test_Add(&test_tqEntryD, 1);

    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEntry(&test_tqEntryA, 1);
    test_AssertEntry(&test_tqEntryB, 2);
    test_AssertEnd();
}

static void test_TwoWithGap_InsertSameAsFirst(void) {
    setUp("test_TwoWithGap_InsertSameAsFirst");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 2);

    test_Add(&test_tqEntryD, 2);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryD, 0);
    test_AssertEntry(&test_tqEntryB, 2);
    test_AssertEnd();
}

static void test_TwoWithGap_InsertMiddle(void) {
    setUp("test_TwoWithGap_InsertMiddle");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 2);

    test_Add(&test_tqEntryD, 3);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEntry(&test_tqEntryB, 1);
    test_AssertEnd();
}

static void test_TwoWithGap_InsertSameAsLast(void) {
    setUp("test_TwoWithGap_InsertSameAsLast");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 2);

    test_Add(&test_tqEntryD, 4);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 2);
    test_AssertEntry(&test_tqEntryD, 0);
    test_AssertEnd();
}

static void test_TwoWithGap_InsertAfter(void) {
    setUp("test_TwoWithGap_InsertAfter");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 2);

    test_Add(&test_tqEntryD, 5);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 2);
    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEnd();
}

static void test_TwoInc_InsertBefore(void) {
    setUp("test_TwoInc_InsertBefore");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);

    test_Add(&test_tqEntryD, 1);

    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEntry(&test_tqEntryA, 1);
    test_AssertEntry(&test_tqEntryB, 3);
    test_AssertEnd();
}

static void test_TwoInc_InsertSameAsFirst(void) {
    setUp("test_TwoInc_InsertSameAsFirst");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);

    test_Add(&test_tqEntryD, 2);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryD, 0);
    test_AssertEntry(&test_tqEntryB, 3);
    test_AssertEnd();
}

static void test_TwoInc_InsertMiddle(void) {
    setUp("test_TwoInc_InsertMiddle");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);

    test_Add(&test_tqEntryD, 3);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEntry(&test_tqEntryB, 2);
    test_AssertEnd();
}

static void test_TwoInc_InsertMiddle2(void) {
    setUp("test_TwoInc_InsertMiddle");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);

    test_Add(&test_tqEntryD, 4);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryD, 2);
    test_AssertEntry(&test_tqEntryB, 1);
    test_AssertEnd();
}

static void test_TwoInc_InsertSameAsLast(void) {
    setUp("test_TwoInc_InsertSameAsLast");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);

    test_Add(&test_tqEntryD, 5);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 3);
    test_AssertEntry(&test_tqEntryD, 0);
    test_AssertEnd();
}

static void test_TwoInc_InsertAfter(void) {
    setUp("test_TwoWithGap_InsertAfter");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);

    test_Add(&test_tqEntryD, 6);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 3);
    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEnd();
}

static void test_TwoIncWithGap_InsertBefore(void) {
    setUp("test_TwoIncWithGap_InsertBefore");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 4);

    test_Add(&test_tqEntryD, 1);

    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEntry(&test_tqEntryA, 1);
    test_AssertEntry(&test_tqEntryB, 4);
    test_AssertEnd();
}

static void test_TwoIncWithGap_InsertSameAsFirst(void) {
    setUp("test_TwoIncWithGap_InsertSameAsFirst");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 4);

    test_Add(&test_tqEntryD, 2);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryD, 0);
    test_AssertEntry(&test_tqEntryB, 4);
    test_AssertEnd();
}

static void test_TwoIncWithGap_InsertMiddle(void) {
    setUp("test_TwoInc_InsertMiddle");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 4);

    test_Add(&test_tqEntryD, 3);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEntry(&test_tqEntryB, 3);
    test_AssertEnd();
}

static void test_TwoIncWithGap_InsertMiddle2(void) {
    setUp("test_TwoInc_InsertMiddle");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 4);

    test_Add(&test_tqEntryD, 4);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryD, 2);
    test_AssertEntry(&test_tqEntryB, 2);
    test_AssertEnd();
}

static void test_TwoIncWithGap_InsertMiddle3(void) {
    setUp("test_TwoInc_InsertMiddle");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 4);

    test_Add(&test_tqEntryD, 5);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryD, 3);
    test_AssertEntry(&test_tqEntryB, 1);
    test_AssertEnd();
}

static void test_TwoIncWithGap_InsertSameAsLast(void) {
    setUp("test_TwoInc_InsertSameAsLast");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 4);

    test_Add(&test_tqEntryD, 6);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 4);
    test_AssertEntry(&test_tqEntryD, 0);
    test_AssertEnd();
}

static void test_TwoIncWithGap_InsertAfter(void) {
    setUp("test_TwoWithGap_InsertAfter");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 4);

    test_Add(&test_tqEntryD, 7);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 4);
    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEnd();
}

static void test_ThreeSame_InsertBefore(void) {
    setUp("test_ThreeSame_InsertBefore");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 0);
    test_AddEntry(&test_tqEntryC, 0);

    test_Add(&test_tqEntryD, 1);

    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEntry(&test_tqEntryA, 1);
    test_AssertEntry(&test_tqEntryB, 0);
    test_AssertEntry(&test_tqEntryC, 0);
    test_AssertEnd();
}

static void test_ThreeSame_InsertMatching(void) {
    setUp("test_ThreeSame_InsertMatching");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 0);
    test_AddEntry(&test_tqEntryC, 0);

    test_Add(&test_tqEntryD, 2);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 0);
    test_AssertEntry(&test_tqEntryC, 0);
    test_AssertEntry(&test_tqEntryD, 0);
    test_AssertEnd();
}

static void test_ThreeSame_InsertAfter(void) {
    setUp("test_ThreeSame_InsertAfter");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 0);
    test_AddEntry(&test_tqEntryC, 0);

    test_Add(&test_tqEntryD, 3);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 0);
    test_AssertEntry(&test_tqEntryC, 0);
    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEnd();
}

static void test_ThreeInc_InsertBefore(void) {
    setUp("test_ThreeInc_InsertBefore");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);
    test_AddEntry(&test_tqEntryC, 4);

    test_Add(&test_tqEntryD, 1);

    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEntry(&test_tqEntryA, 1);
    test_AssertEntry(&test_tqEntryB, 3);
    test_AssertEntry(&test_tqEntryC, 4);
    test_AssertEnd();
}

static void test_ThreeInc_InsertSameAsFirst(void) {
    setUp("test_ThreeInc_SameAsFirst");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);
    test_AddEntry(&test_tqEntryC, 4);

    test_Add(&test_tqEntryD, 2);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryD, 0);
    test_AssertEntry(&test_tqEntryB, 3);
    test_AssertEntry(&test_tqEntryC, 4);
    test_AssertEnd();
}

static void test_ThreeInc_InsertFirstMiddle(void) {
    setUp("test_ThreeInc_FirstMiddle");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);
    test_AddEntry(&test_tqEntryC, 4);

    test_Add(&test_tqEntryD, 3);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEntry(&test_tqEntryB, 2);
    test_AssertEntry(&test_tqEntryC, 4);
    test_AssertEnd();
}

static void test_ThreeInc_InsertFirstMiddle2(void) {
    setUp("test_ThreeInc_FirstMiddle2");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);
    test_AddEntry(&test_tqEntryC, 4);

    test_Add(&test_tqEntryD, 4);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryD, 2);
    test_AssertEntry(&test_tqEntryB, 1);
    test_AssertEntry(&test_tqEntryC, 4);
    test_AssertEnd();
}

static void test_ThreeInc_InsertSameAsSecond(void) {
    setUp("test_ThreeInc_InsertSameAsSecond");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);
    test_AddEntry(&test_tqEntryC, 4);

    test_Add(&test_tqEntryD, 5);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 3);
    test_AssertEntry(&test_tqEntryD, 0);
    test_AssertEntry(&test_tqEntryC, 4);
    test_AssertEnd();
}

static void test_ThreeInc_InsertSecondMiddle(void) {
    setUp("test_ThreeInc_InsertSecondMiddle");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);
    test_AddEntry(&test_tqEntryC, 4);

    test_Add(&test_tqEntryD, 6);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 3);
    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEntry(&test_tqEntryC, 3);
    test_AssertEnd();
}

static void test_ThreeInc_InsertSecondMiddle2(void) {
    setUp("test_ThreeInc_InsertSecondMiddle2");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);
    test_AddEntry(&test_tqEntryC, 4);

    test_Add(&test_tqEntryD, 7);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 3);
    test_AssertEntry(&test_tqEntryD, 2);
    test_AssertEntry(&test_tqEntryC, 2);
    test_AssertEnd();
}

static void test_ThreeInc_InsertSecondMiddle3(void) {
    setUp("test_ThreeInc_InsertSecondMiddle3");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);
    test_AddEntry(&test_tqEntryC, 4);

    test_Add(&test_tqEntryD, 8);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 3);
    test_AssertEntry(&test_tqEntryD, 3);
    test_AssertEntry(&test_tqEntryC, 1);
    test_AssertEnd();
}

static void test_ThreeInc_InsertSameAsLast(void) {
    setUp("test_ThreeInc_InsertSameAsLast");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);
    test_AddEntry(&test_tqEntryC, 4);

    test_Add(&test_tqEntryD, 9);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 3);
    test_AssertEntry(&test_tqEntryC, 4);
    test_AssertEntry(&test_tqEntryD, 0);
    test_AssertEnd();
}

static void test_ThreeInc_InsertAfter(void) {
    setUp("test_ThreeInc_InsertSameAsLast");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);
    test_AddEntry(&test_tqEntryC, 4);

    test_Add(&test_tqEntryD, 10);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 3);
    test_AssertEntry(&test_tqEntryC, 4);
    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEnd();
}

static void test_ThreeIncFirstTwoSame_InsertBefore(void) {
    setUp("test_ThreeIncFirstTwoSame_InsertBefore");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 0);
    test_AddEntry(&test_tqEntryC, 3);

    test_Add(&test_tqEntryD, 1);

    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEntry(&test_tqEntryA, 1);
    test_AssertEntry(&test_tqEntryB, 0);
    test_AssertEntry(&test_tqEntryC, 3);
    test_AssertEnd();
}

static void test_ThreeIncFirstTwoSame_InsertSameAsFirst(void) {
    setUp("test_ThreeIncFirstTwoSame_InsertSameAsFirst");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 0);
    test_AddEntry(&test_tqEntryC, 3);

    test_Add(&test_tqEntryD, 2);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 0);
    test_AssertEntry(&test_tqEntryD, 0);
    test_AssertEntry(&test_tqEntryC, 3);
    test_AssertEnd();
}

static void test_ThreeIncFirstTwoSame_InsertMiddle(void) {
    setUp("test_ThreeIncFirstTwoSame_InsertMiddle");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 0);
    test_AddEntry(&test_tqEntryC, 3);

    test_Add(&test_tqEntryD, 3);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 0);
    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEntry(&test_tqEntryC, 2);
    test_AssertEnd();
}

static void test_ThreeIncFirstTwoSame_InsertMiddle2(void) {
    setUp("test_ThreeIncFirstTwoSame_InsertMiddle2");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 0);
    test_AddEntry(&test_tqEntryC, 3);

    test_Add(&test_tqEntryD, 4);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 0);
    test_AssertEntry(&test_tqEntryD, 2);
    test_AssertEntry(&test_tqEntryC, 1);
    test_AssertEnd();
}

static void test_ThreeIncFirstTwoSame_InsertSameAsLast(void) {
    setUp("test_ThreeIncFirstTwoSame_InsertSameAsLast");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 0);
    test_AddEntry(&test_tqEntryC, 3);

    test_Add(&test_tqEntryD, 5);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 0);
    test_AssertEntry(&test_tqEntryC, 3);
    test_AssertEntry(&test_tqEntryD, 0);
    test_AssertEnd();
}

static void test_ThreeIncFirstTwoSame_InsertAfter(void) {
    setUp("test_ThreeIncFirstTwoSame_InsertAfter");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 0);
    test_AddEntry(&test_tqEntryC, 3);

    test_Add(&test_tqEntryD, 6);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 0);
    test_AssertEntry(&test_tqEntryC, 3);
    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEnd();
}

static void test_ThreeIncLastTwoSame_InsertBefore(void) {
    setUp("test_ThreeIncLastTwoSame_InsertBefore");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);
    test_AddEntry(&test_tqEntryC, 0);

    test_Add(&test_tqEntryD, 1);

    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEntry(&test_tqEntryA, 1);
    test_AssertEntry(&test_tqEntryB, 3);
    test_AssertEntry(&test_tqEntryC, 0);
    test_AssertEnd();
}

static void test_ThreeIncLastTwoSame_InsertSameAsFirst(void) {
    setUp("test_ThreeIncLastTwoSame_InsertSameAsFirst");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);
    test_AddEntry(&test_tqEntryC, 0);

    test_Add(&test_tqEntryD, 2);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryD, 0);
    test_AssertEntry(&test_tqEntryB, 3);
    test_AssertEntry(&test_tqEntryC, 0);
    test_AssertEnd();
}

static void test_ThreeIncLastTwoSame_InsertMiddle(void) {
    setUp("test_ThreeIncLastTwoSame_InsertMiddle");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);
    test_AddEntry(&test_tqEntryC, 0);

    test_Add(&test_tqEntryD, 3);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEntry(&test_tqEntryB, 2);
    test_AssertEntry(&test_tqEntryC, 0);
    test_AssertEnd();
}

static void test_ThreeIncLastTwoSame_InsertMiddle2(void) {
    setUp("test_ThreeIncLastTwoSame_InsertMiddle");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);
    test_AddEntry(&test_tqEntryC, 0);

    test_Add(&test_tqEntryD, 4);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryD, 2);
    test_AssertEntry(&test_tqEntryB, 1);
    test_AssertEntry(&test_tqEntryC, 0);
    test_AssertEnd();
}

static void test_ThreeIncLastTwoSame_InsertSameAsLast(void) {
    setUp("test_ThreeIncLastTwoSame_InsertSameAsLast");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);
    test_AddEntry(&test_tqEntryC, 0);

    test_Add(&test_tqEntryD, 5);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 3);
    test_AssertEntry(&test_tqEntryC, 0);
    test_AssertEntry(&test_tqEntryD, 0);
    test_AssertEnd();
}

static void test_ThreeIncLastTwoSame_InsertAfter(void) {
    setUp("test_ThreeIncLastTwoSame_InsertSameAsLast");
    test_AddEntry(&test_tqEntryA, 2);
    test_AddEntry(&test_tqEntryB, 3);
    test_AddEntry(&test_tqEntryC, 0);

    test_Add(&test_tqEntryD, 6);

    test_AssertEntry(&test_tqEntryA, 2);
    test_AssertEntry(&test_tqEntryB, 3);
    test_AssertEntry(&test_tqEntryC, 0);
    test_AssertEntry(&test_tqEntryD, 1);
    test_AssertEnd();
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_Init();
    test_Empty();

    test_One_InsertBefore();
    test_One_InsertMatching();
    test_One_InsertAfter();

    test_TwoSame_InsertBefore();
    test_TwoSame_InsertSame();
    test_TwoSame_InsertAfter();

    test_TwoWithGap_InsertBefore();
    test_TwoWithGap_InsertSameAsFirst();
    test_TwoWithGap_InsertMiddle();
    test_TwoWithGap_InsertSameAsLast();
    test_TwoWithGap_InsertAfter();

    test_TwoInc_InsertBefore();
    test_TwoInc_InsertSameAsFirst();
    test_TwoInc_InsertMiddle();
    test_TwoInc_InsertMiddle2();
    test_TwoInc_InsertSameAsLast();
    test_TwoInc_InsertAfter();

    test_TwoIncWithGap_InsertBefore();
    test_TwoIncWithGap_InsertSameAsFirst();
    test_TwoIncWithGap_InsertMiddle();
    test_TwoIncWithGap_InsertMiddle2();
    test_TwoIncWithGap_InsertMiddle3();
    test_TwoIncWithGap_InsertSameAsLast();
    test_TwoIncWithGap_InsertAfter();

    test_ThreeSame_InsertBefore();
    test_ThreeSame_InsertMatching();
    test_ThreeSame_InsertAfter();

    test_ThreeInc_InsertBefore();
    test_ThreeInc_InsertSameAsFirst();
    test_ThreeInc_InsertFirstMiddle();
    test_ThreeInc_InsertFirstMiddle2();
    test_ThreeInc_InsertSameAsSecond();
    test_ThreeInc_InsertSecondMiddle();
    test_ThreeInc_InsertSecondMiddle2();
    test_ThreeInc_InsertSecondMiddle3();
    test_ThreeInc_InsertSameAsLast();
    test_ThreeInc_InsertAfter();

    test_ThreeIncFirstTwoSame_InsertBefore();
    test_ThreeIncFirstTwoSame_InsertSameAsFirst();
    test_ThreeIncFirstTwoSame_InsertMiddle();
    test_ThreeIncFirstTwoSame_InsertMiddle2();
    test_ThreeIncFirstTwoSame_InsertSameAsLast();
    test_ThreeIncFirstTwoSame_InsertAfter();

    test_ThreeIncLastTwoSame_InsertBefore();
    test_ThreeIncLastTwoSame_InsertSameAsFirst();
    test_ThreeIncLastTwoSame_InsertMiddle();
    test_ThreeIncLastTwoSame_InsertMiddle2();
    test_ThreeIncLastTwoSame_InsertSameAsLast();
    test_ThreeIncLastTwoSame_InsertAfter();

    Assert_Save();
    return 0;
}