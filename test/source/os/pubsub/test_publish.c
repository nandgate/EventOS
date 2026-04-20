#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

Mock_Void(hal_CriticalBegin);
Mock_Void(hal_CriticalEnd);
Mock_Value2(os_context_t, os_Do, os_action_t, uint32_t);
Mock_Void2(os_DoWith, os_action_t, os_context_t);
Mock_Void1(os_NullAction, os_context_t);
Mock_Void1(test_Action, os_context_t);
Mock_Void1(test_Action2, os_context_t);

#define TEST_TOPIC1         42
#define TEST_TOPIC2         24
#define TEST_TOPIC3         44
#define TEST_CONTEXT_SIZE   8

os_subscription_t *os_subscriptions;
os_subscription_t test_subscription;
os_subscription_t test_subscription2;
int test_context;

static void setUp(void)
{
    Test_Init();
    Mock_Reset(hal_CriticalBegin);
    Mock_Reset(hal_CriticalEnd);
    Mock_Reset(os_Do);
    Mock_Reset(os_DoWith);
    Mock_Reset(os_NullAction);
    Mock_Reset(test_Action);
    Mock_Reset(test_Action2);

    Mock_Returns(os_Do, &test_context);
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

    os_context_t result = os_Publish(TEST_TOPIC2, TEST_CONTEXT_SIZE);

    Assert_CalledOnce(os_Do);
    Assert_Called2(os_Do, test_Action2, TEST_CONTEXT_SIZE);
    Assert_Equals(&test_context, result);
}

static void test_TopicWithoutSubscription(void) {
    setUp();

    os_context_t result = os_Publish(TEST_TOPIC3, TEST_CONTEXT_SIZE);

    Assert_CalledOnce(os_Do);
    Assert_Called2(os_Do, os_NullAction, TEST_CONTEXT_SIZE);
    Assert_Equals(&test_context, result);
}

static void test_TopicWithMultipleSubscriptions(void) {
    setUp();
    // Two subscriptions, same topic.
    test_subscription.topic = TEST_TOPIC1;
    test_subscription.action = test_Action;
    test_subscription2.topic = TEST_TOPIC1;
    test_subscription2.action = test_Action2;

    os_context_t result = os_Publish(TEST_TOPIC1, TEST_CONTEXT_SIZE);

    Assert_CalledOnce(os_Do);
    Assert_Called2(os_Do, test_Action, TEST_CONTEXT_SIZE);

    Assert_CalledOnce(os_DoWith);
    Assert_Called2(os_DoWith, test_Action2, result);
    Assert_Equals(&test_context, result);
}

static void test_WrapsListWalkInCriticalSection(void) {
    setUp();

    os_Publish(TEST_TOPIC2, TEST_CONTEXT_SIZE);

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
