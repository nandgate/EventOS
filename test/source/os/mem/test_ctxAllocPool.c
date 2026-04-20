#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(16);

Mock_Void(hal_CriticalBegin);
Mock_Void(hal_CriticalEnd);

#ifndef OS_CTX_SMALL_SLOT
#define OS_CTX_SMALL_SLOT  16
#endif
#ifndef OS_CTX_SMALL_COUNT
#define OS_CTX_SMALL_COUNT 4
#endif
#ifndef OS_CTX_LARGE_SLOT
#define OS_CTX_LARGE_SLOT  64
#endif
#ifndef OS_CTX_LARGE_COUNT
#define OS_CTX_LARGE_COUNT 4
#endif

#define CTX_SLOT_WORDS(bytes) (((bytes) + sizeof(uint32_t) - 1) / sizeof(uint32_t))

uint32_t    os_ctxSmallPool[OS_CTX_SMALL_COUNT][CTX_SLOT_WORDS(OS_CTX_SMALL_SLOT)];
uint32_t    os_ctxLargePool[OS_CTX_LARGE_COUNT][CTX_SLOT_WORDS(OS_CTX_LARGE_SLOT)];
os_ctx_t   *os_ctxSmallFreeList;
os_ctx_t   *os_ctxLargeFreeList;
uint32_t    os_ctxInUseCount;
uint32_t    os_ctxHighWaterMark;

#define TEST_SMALL_SIZE  OS_CTX_SMALL_SLOT
#define TEST_LARGE_SIZE  (OS_CTX_SMALL_SLOT + 1)
#define TEST_TOO_BIG     (OS_CTX_LARGE_SLOT + 1)

static void setUp(void)
{
    Test_Init();
    Mock_Reset(hal_CriticalBegin);
    Mock_Reset(hal_CriticalEnd);
    os_CtxAllocInit();
}

static void test_AllocSmall_ReturnsNonNull(void)
{
    setUp();

    os_ctx_t *result = os_CtxAlloc(TEST_SMALL_SIZE);

    Assert_IsNotNull(result);
}

static void test_AllocSmall_ReturnsDistinctSlots(void)
{
    setUp();

    os_ctx_t *s1 = os_CtxAlloc(TEST_SMALL_SIZE);
    os_ctx_t *s2 = os_CtxAlloc(TEST_SMALL_SIZE);

    Assert_IsNotNull(s1);
    Assert_IsNotNull(s2);
    Assert_True(s1 != s2);
}

static void test_AllocSmall_Exhaustion(void)
{
    setUp();
    os_ctx_t *s1 = os_CtxAlloc(TEST_SMALL_SIZE);
    os_ctx_t *s2 = os_CtxAlloc(TEST_SMALL_SIZE);
    os_ctx_t *s3 = os_CtxAlloc(TEST_SMALL_SIZE);
    os_ctx_t *s4 = os_CtxAlloc(TEST_SMALL_SIZE);
    (void)s1; (void)s2; (void)s3; (void)s4;

    os_ctx_t *overflow = os_CtxAlloc(TEST_SMALL_SIZE);

    Assert_IsNull(overflow);
}

static void test_AllocLarge_ReturnsNonNull(void)
{
    setUp();

    os_ctx_t *result = os_CtxAlloc(TEST_LARGE_SIZE);

    Assert_IsNotNull(result);
}

static void test_AllocLarge_Exhaustion(void)
{
    setUp();
    os_ctx_t *l1 = os_CtxAlloc(TEST_LARGE_SIZE);
    os_ctx_t *l2 = os_CtxAlloc(TEST_LARGE_SIZE);
    os_ctx_t *l3 = os_CtxAlloc(TEST_LARGE_SIZE);
    os_ctx_t *l4 = os_CtxAlloc(TEST_LARGE_SIZE);
    (void)l1; (void)l2; (void)l3; (void)l4;

    os_ctx_t *overflow = os_CtxAlloc(TEST_LARGE_SIZE);

    Assert_IsNull(overflow);
}

static void test_AllocTooBig_ReturnsNull(void)
{
    setUp();

    os_ctx_t *result = os_CtxAlloc(TEST_TOO_BIG);

    Assert_IsNull(result);
}

static void test_Free_AllowsReuse(void)
{
    setUp();
    os_ctx_t *s1 = os_CtxAlloc(TEST_SMALL_SIZE);
    os_ctx_t *s2 = os_CtxAlloc(TEST_SMALL_SIZE);
    os_ctx_t *s3 = os_CtxAlloc(TEST_SMALL_SIZE);
    os_ctx_t *s4 = os_CtxAlloc(TEST_SMALL_SIZE);
    (void)s1; (void)s2; (void)s3;

    os_CtxFree(s4);
    os_ctx_t *reused = os_CtxAlloc(TEST_SMALL_SIZE);

    Assert_IsNotNull(reused);
    Assert_Equals(s4, reused);
}

static void test_SmallExhaustion_DoesNotAffectLarge(void)
{
    setUp();
    os_CtxAlloc(TEST_SMALL_SIZE);
    os_CtxAlloc(TEST_SMALL_SIZE);
    os_CtxAlloc(TEST_SMALL_SIZE);
    os_CtxAlloc(TEST_SMALL_SIZE);

    os_ctx_t *large = os_CtxAlloc(TEST_LARGE_SIZE);

    Assert_IsNotNull(large);
}

static void test_InUseCount(void)
{
    setUp();
    os_CtxAlloc(TEST_SMALL_SIZE);
    os_CtxAlloc(TEST_LARGE_SIZE);

    Assert_Equals(2, os_ctxInUseCount);
}

static void test_FreeDecrementsInUseCount(void)
{
    setUp();
    os_ctx_t *s = os_CtxAlloc(TEST_SMALL_SIZE);
    os_CtxAlloc(TEST_LARGE_SIZE);

    os_CtxFree(s);

    Assert_Equals(1, os_ctxInUseCount);
}

static void test_HighWaterMark(void)
{
    setUp();
    os_CtxAlloc(TEST_SMALL_SIZE);
    os_ctx_t *s2 = os_CtxAlloc(TEST_SMALL_SIZE);
    os_CtxAlloc(TEST_SMALL_SIZE);
    os_CtxFree(s2);

    Assert_Equals(3, os_ctxHighWaterMark);
}

static void test_AllocWrapsInCriticalSection(void)
{
    setUp();

    os_CtxAlloc(TEST_SMALL_SIZE);

    Assert_CalledOnce(hal_CriticalBegin);
    Assert_CalledOnce(hal_CriticalEnd);
}

static void test_FreeWrapsInCriticalSection(void)
{
    setUp();
    os_ctx_t *s = os_CtxAlloc(TEST_SMALL_SIZE);
    Mock_Reset(hal_CriticalBegin);
    Mock_Reset(hal_CriticalEnd);

    os_CtxFree(s);

    Assert_CalledOnce(hal_CriticalBegin);
    Assert_CalledOnce(hal_CriticalEnd);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_AllocSmall_ReturnsNonNull();
    test_AllocSmall_ReturnsDistinctSlots();
    test_AllocSmall_Exhaustion();
    test_AllocLarge_ReturnsNonNull();
    test_AllocLarge_Exhaustion();
    test_AllocTooBig_ReturnsNull();
    test_Free_AllowsReuse();
    test_SmallExhaustion_DoesNotAffectLarge();
    test_InUseCount();
    test_FreeDecrementsInUseCount();
    test_HighWaterMark();
    test_AllocWrapsInCriticalSection();
    test_FreeWrapsInCriticalSection();

    Assert_Save();
    return 0;
}
