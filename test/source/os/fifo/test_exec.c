#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(5);

#define TEST_UNSET          0x42
#define TEST_CONTEXT_SIZE   42

Mock_Void(hal_CriticalBegin);
Mock_Void(hal_CriticalEnd);
Mock_Void1(os_ActionBegin, os_action_t);
Mock_Void1(os_ActionEnd, os_action_t);
Mock_Void1(os_ContextRelease, os_ctx_t *);
Mock_Void1(os_EntryFree, os_entry_t *);
Mock_Value(os_entry_t *, os_FifoRemove);
Mock_Void1(test_Action, os_context_t);

os_entry_t test_actionEntry;
os_ctx_t test_contextEntry;
os_ctx_t test_heldCtxEntry;
os_ctx_t test_heldCtxEntry2;
os_ctx_t *os_heldCtx;

static void setUp(void)
{
    Test_Init();
    Mock_Reset(hal_CriticalBegin);
    Mock_Reset(hal_CriticalEnd);
    Mock_Reset(os_ActionBegin);
    Mock_Reset(os_ActionEnd);
    Mock_Reset(os_ContextRelease);
    Mock_Reset(os_EntryFree);
    Mock_Reset(os_FifoRemove);
    Mock_Reset(test_Action);

    test_contextEntry.count = 1;
    test_contextEntry.size = TEST_CONTEXT_SIZE;
    test_contextEntry.next = NULL;

    test_heldCtxEntry.next = NULL;
    test_heldCtxEntry2.next = NULL;

    test_actionEntry.ctx = &test_contextEntry;
    test_actionEntry.next = NULL;
    test_actionEntry.action = test_Action;

    os_heldCtx = NULL;
}

static void test_ExecFifoEmpty(void)
{
    setUp();
    Mock_Returns(os_FifoRemove, NULL);

    os_Exec();

    Assert_CalledOnce(os_FifoRemove);

    Assert_NotCalled(test_Action);
}

static void test_ExecFifoNotEmpty(void) {
    setUp();
    Mock_Returns(os_FifoRemove, &test_actionEntry);

    os_Exec();

    Assert_CalledOnce(test_Action);
    Assert_Called1(test_Action, &test_contextEntry.data);

    Assert_Called1(os_ContextRelease, &test_contextEntry);
    Assert_Called1(os_EntryFree, &test_actionEntry);
}

static void test_ExecNoContext(void) {
    setUp();
    test_actionEntry.ctx = NULL;
    Mock_Returns(os_FifoRemove, &test_actionEntry);

    os_Exec();

    Assert_CalledOnce(test_Action);
    Assert_Called1(test_Action, NULL);

    Assert_NotCalled(os_ContextRelease);
    Assert_CalledOnce(os_EntryFree);
}

static void test_ExecCallsActionBeginWithAction(void) {
    setUp();
    Mock_Returns(os_FifoRemove, &test_actionEntry);

    os_Exec();

    Assert_Called1(os_ActionBegin, test_Action);
}

static void test_ExecCallsActionEndWithAction(void) {
    setUp();
    Mock_Returns(os_FifoRemove, &test_actionEntry);

    os_Exec();

    Assert_Called1(os_ActionEnd, test_Action);
}

static void test_ExecReleasesHeldCtx(void) {
    setUp();
    Mock_Returns(os_FifoRemove, &test_actionEntry);
    os_heldCtx = &test_heldCtxEntry;

    os_Exec();

    Assert_CallCount(2, os_ContextRelease);
    Assert_CalledFirst1(os_ContextRelease, &test_heldCtxEntry);
    Assert_CalledLast1(os_ContextRelease, &test_contextEntry);
    Assert_IsNull(os_heldCtx);
}

static void test_ExecReleasesAllHeldCtxs(void) {
    setUp();
    Mock_Returns(os_FifoRemove, &test_actionEntry);
    // Held list: heldCtxEntry -> heldCtxEntry2
    test_heldCtxEntry.next = &test_heldCtxEntry2;
    os_heldCtx = &test_heldCtxEntry;

    os_Exec();

    Assert_CallCount(3, os_ContextRelease);
    Assert_CalledFirst1(os_ContextRelease, &test_heldCtxEntry);
    Assert_CalledN1(2, os_ContextRelease, &test_heldCtxEntry2);
    Assert_CalledLast1(os_ContextRelease, &test_contextEntry);
    Assert_IsNull(os_heldCtx);
}

static void test_ExecLeavesHeldCtxUnchangedWhenNull(void) {
    setUp();
    Mock_Returns(os_FifoRemove, &test_actionEntry);

    os_Exec();

    Assert_CallCount(1, os_ContextRelease);
    Assert_Called1(os_ContextRelease, &test_contextEntry);
    Assert_IsNull(os_heldCtx);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_ExecFifoEmpty();
    test_ExecFifoNotEmpty();
    test_ExecNoContext();
    test_ExecCallsActionBeginWithAction();
    test_ExecCallsActionEndWithAction();
    test_ExecReleasesHeldCtx();
    test_ExecReleasesAllHeldCtxs();
    test_ExecLeavesHeldCtxUnchangedWhenNull();

    Assert_Save();
    return 0;
}
