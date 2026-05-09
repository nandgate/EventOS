#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

Mock_Void(os_CtxAllocInit);
Mock_Void(os_EntryAllocInit);
Mock_Void(os_SubAllocInit);
Mock_Void(os_TimerInit);

extern os_actionFifo_t    os_actionFifo;
extern os_ctx_t          *os_heldCtx;
extern os_subscription_t *os_subscriptions;
extern os_entry_t        *os_tQueue;

static void setUp(void)
{
    Test_Init();
    Mock_Reset(os_CtxAllocInit);
    Mock_Reset(os_EntryAllocInit);
    Mock_Reset(os_SubAllocInit);
    Mock_Reset(os_TimerInit);

    os_heldCtx = (os_ctx_t *)0xDEAD;
    os_subscriptions = (os_subscription_t *)0xDEAD;
    os_actionFifo.os_fifoHead = (os_entry_t *)0xDEAD;
    os_actionFifo.os_fifoTail = (os_entry_t *)0xDEAD;
}

static void test_InitializesSubsystems(void)
{
    setUp();

    os_Init();

    Assert_CalledOnce(os_EntryAllocInit);
    Assert_CalledOnce(os_SubAllocInit);
    Assert_CalledOnce(os_CtxAllocInit);
    Assert_CalledOnce(os_TimerInit);
}

static void test_InitializesGlobalState(void)
{
    setUp();

    os_Init();

    Assert_IsNull(os_heldCtx);
    Assert_IsNull(os_subscriptions);
    Assert_IsNull(os_actionFifo.os_fifoHead);
    Assert_IsNull(os_actionFifo.os_fifoTail);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_InitializesSubsystems();
    test_InitializesGlobalState();

    Assert_Save();
    return 0;
}
