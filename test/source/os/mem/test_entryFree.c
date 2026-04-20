#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

Mock_Void(hal_CriticalBegin);
Mock_Void(hal_CriticalEnd);
Mock_Void1(os_Fail, os_fail_t);

os_entry_t *os_entryFreeList;
uint32_t    os_entryInUseCount;

static void setUp(void)
{
    Test_Init();
    Mock_Reset(hal_CriticalBegin);
    Mock_Reset(hal_CriticalEnd);
    Mock_Reset(os_Fail);
}

static void test_NewEntryBecomesFreeListHead(void)
{
    setUp();
    os_entry_t entry;
    os_entryFreeList = NULL;

    os_EntryFree(&entry);

    Assert_Equals(&entry, os_entryFreeList);
}

static void test_LinksEntryToPreviousHead(void)
{
    setUp();
    os_entry_t entry;
    os_entry_t oldHead;
    os_entryFreeList = &oldHead;

    os_EntryFree(&entry);

    Assert_Equals(&oldHead, entry.next);
}

static void test_DecrementsInUseCount(void)
{
    setUp();
    os_entry_t entry;
    os_entryFreeList = NULL;
    os_entryInUseCount = 7;

    os_EntryFree(&entry);

    Assert_Equals(6, os_entryInUseCount);
}

static void test_Null_CallsFail(void)
{
    setUp();

    os_EntryFree(NULL);

    Assert_CalledOnce(os_Fail);
    Assert_Called1(os_Fail, OS_FAIL_ENTRY_FREE);
}

static void test_WrapsInCriticalSection(void)
{
    setUp();
    os_entry_t entry;
    os_entryFreeList = NULL;

    os_EntryFree(&entry);

    Assert_CalledOnce(hal_CriticalBegin);
    Assert_CalledOnce(hal_CriticalEnd);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_NewEntryBecomesFreeListHead();
    test_LinksEntryToPreviousHead();
    test_DecrementsInUseCount();
    test_Null_CallsFail();
    test_WrapsInCriticalSection();

    Assert_Save();
    return 0;
}
