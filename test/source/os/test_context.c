#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

#define TEST_UNSET          0x42
#define TEST_CONTEXT_SIZE   42

os_ctx_t test_contextEntry;

Mock_Value1(void *, os_MemAlloc, uint32_t);
Mock_Void1(os_MemFree, void*);

static void setUp(void)
{
    Mock_Reset(os_MemAlloc);
    Mock_Reset(os_MemFree);

    Mock_Returns(os_MemAlloc, &test_contextEntry);

    test_contextEntry.size= TEST_UNSET;
    test_contextEntry.count = TEST_UNSET;
}

static void test_ContextNew(void)
{
    setUp();

    os_ctx_t *result = os_ContextNew(TEST_CONTEXT_SIZE);

    Assert_Equals(&test_contextEntry, result);
    Assert_Equals(1, result->count);
    Assert_Equals(TEST_CONTEXT_SIZE, result->size);

    Assert_CalledOnce(os_MemAlloc);
    Assert_Called1(os_MemAlloc, sizeof(os_ctx_t) + TEST_CONTEXT_SIZE);
}

static void test_ContextUse_SillInUse(void)
{
    setUp();
    test_contextEntry.count= 2;

    os_ContextUse(&test_contextEntry);

    Assert_Equals(1, test_contextEntry.count);
    Assert_NotCalled(os_MemFree);
}

static void test_ContextUse_LastUse(void)
{
    setUp();
    test_contextEntry.count= 1;
    test_contextEntry.size= TEST_CONTEXT_SIZE;

    os_ContextUse(&test_contextEntry);

    Assert_Equals(0, test_contextEntry.count);

    Assert_CalledOnce(os_MemFree);
    Assert_Called1(os_MemFree, &test_contextEntry);
}

static void test_ContextReuse_FindsContextEntry(void)
{
    setUp();

    os_ctx_t *result = os_ContextReuse(&test_contextEntry.data);

    Assert_Equals(&test_contextEntry, result);
}

static void test_ContextReuse_AdjustUseCount(void)
{
    setUp();
    test_contextEntry.count = 3;

    os_ContextReuse(&test_contextEntry.data);

    Assert_Equals(4, test_contextEntry.count);
}

static void test_GetCtx(void)
{
    setUp();

    os_ctx_t *result = os_GetCtx(test_contextEntry.data);

    Assert_Equals(&test_contextEntry, result);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_ContextNew();

    test_ContextUse_SillInUse();
    test_ContextUse_LastUse();

    test_ContextReuse_FindsContextEntry();
    test_ContextReuse_AdjustUseCount();

    test_GetCtx();

    Assert_Save();
    return 0;
}
