#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

Mock_Void(hal_CriticalBegin);
Mock_Void(hal_CriticalEnd);
Mock_Void2(os_DoWith, os_action_t, os_context_t);
Mock_Void1(test_Action, os_context_t);
Mock_Void1(test_Action2, os_context_t);

#define TEST_TOPIC1         42
#define TEST_TOPIC2         24
#define TEST_TOPIC3         44

os_subscription_t *os_subscriptions;
os_subscription_t test_subscription;
os_subscription_t test_subscription2;
int test_context;
int test_context2;

static void setUp(void)
{
    Test_Init();
    Mock_Reset(hal_CriticalBegin);
    Mock_Reset(hal_CriticalEnd);
    Mock_Reset(os_DoWith);
    Mock_Reset(test_Action);
    Mock_Reset(test_Action2);

    os_subscriptions = &test_subscription;

    test_subscription.topic = TEST_TOPIC1;
    test_subscription.action = test_Action;
    test_subscription.next = &test_subscription2;

    test_subscription2.topic = TEST_TOPIC2;
    test_subscription2.action = test_Action2;
    test_subscription2.next = NULL;
}

static void test_TopicWithSubscription(void) {
    setUp();

    os_PublishWith(TEST_TOPIC2, &test_context2);

    Assert_CalledOnce(os_DoWith);
    Assert_Called2(os_DoWith, test_Action2, &test_context2);
}

static void test_TopicWithoutSubscription(void) {
    setUp();

    os_PublishWith(TEST_TOPIC3, &test_context2);

    Assert_NotCalled(os_DoWith);
}

static void test_TopicWithMultipleSubscriptions(void) {
    setUp();
    // Two subscriptions, same topic.
    test_subscription.topic = TEST_TOPIC1;
    test_subscription.action = test_Action;
    test_subscription2.topic = TEST_TOPIC1;
    test_subscription2.action = test_Action2;

    os_PublishWith(TEST_TOPIC1, &test_context);

    Assert_CallCount(2, os_DoWith);
    Assert_Called2(os_DoWith, test_Action, &test_context);
    Assert_Called2(os_DoWith, test_Action2, &test_context);
}

static void test_WrapsListWalkInCriticalSection(void) {
    setUp();

    os_PublishWith(TEST_TOPIC2, &test_context2);

    Assert_CalledOnce(hal_CriticalBegin);
    Assert_CalledOnce(hal_CriticalEnd);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_TopicWithSubscription();
    test_TopicWithoutSubscription();
    test_TopicWithMultipleSubscriptions();
    test_WrapsListWalkInCriticalSection();

    Assert_Save();
    return 0;
}
