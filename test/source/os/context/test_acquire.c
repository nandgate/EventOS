#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

os_ctx_t test_contextEntry;

static void setUp(void)
{
    Test_Init();
}

static void test_NullReturnsNull(void)
{
    setUp();

    os_ctx_t *result = os_ContextAcquire(NULL);

    Assert_IsNull(result);
}

static void test_FindsContextEntry(void)
{
    setUp();

    os_ctx_t *result = os_ContextAcquire(&test_contextEntry.data);

    Assert_Equals(&test_contextEntry, result);
}

static void test_IncrementsUseCount(void)
{
    setUp();
    test_contextEntry.count = 3;

    os_ContextAcquire(&test_contextEntry.data);

    Assert_Equals(4, test_contextEntry.count);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_NullReturnsNull();
    test_FindsContextEntry();
    test_IncrementsUseCount();

    Assert_Save();
    return 0;
}
