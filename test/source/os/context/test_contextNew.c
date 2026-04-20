#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

#define TEST_CONTEXT_SIZE   42

os_ctx_t test_contextEntry;

Mock_Value1(os_ctx_t *, os_CtxAlloc, uint32_t);
Mock_Void1(os_Fail, os_fail_t);

static void setUp(void)
{
    Test_Init();
    Mock_Reset(os_CtxAlloc);
    Mock_Reset(os_Fail);

    Mock_Returns(os_CtxAlloc, &test_contextEntry);

    test_contextEntry.size = 0;
    test_contextEntry.count = 0;
}

static void test_NoContextReturnsNull(void)
{
    setUp();

    os_ctx_t *result = os_ContextNew(OS_NO_CONTEXT);

    Assert_IsNull(result);
    Assert_NotCalled(os_CtxAlloc);
}

static void test_ReturnsAllocatedContext(void)
{
    setUp();

    os_ctx_t *result = os_ContextNew(TEST_CONTEXT_SIZE);

    Assert_Equals(&test_contextEntry, result);
    Assert_Equals(1, result->count);
    Assert_Equals(TEST_CONTEXT_SIZE, result->size);
}

static void test_PassesTotalSizeToAllocator(void)
{
    setUp();

    os_ContextNew(TEST_CONTEXT_SIZE);

    Assert_CalledOnce(os_CtxAlloc);
    Assert_Called1(os_CtxAlloc, sizeof(os_ctx_t) + TEST_CONTEXT_SIZE);
}

static void test_AllocFails(void)
{
    setUp();
    Mock_Returns(os_CtxAlloc, NULL);

    os_ctx_t *result = os_ContextNew(TEST_CONTEXT_SIZE);

    Assert_IsNull(result);
    Assert_CalledOnce(os_Fail);
    Assert_Called1(os_Fail, OS_FAIL_CONTEXT_ALLOCATION);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_NoContextReturnsNull();
    test_ReturnsAllocatedContext();
    test_PassesTotalSizeToAllocator();
    test_AllocFails();

    Assert_Save();
    return 0;
}
