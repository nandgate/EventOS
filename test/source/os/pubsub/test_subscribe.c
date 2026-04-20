#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(4);

Mock_Void(hal_CriticalBegin);
Mock_Void(hal_CriticalEnd);
Mock_Void1(os_Fail, os_fail_t);
Mock_Value(os_subscription_t *, os_SubAlloc);
Mock_Void1(test_Action, os_context_t);
Mock_Void1(test_Action2, os_context_t);

#define TEST_TOPIC1         42
#define TEST_TOPIC2         24

os_subscription_t *os_subscriptions;
os_subscription_t test_subscription;
os_subscription_t test_subscription2;

static void setUp(void)
{
    Test_Init();
    Mock_Reset(hal_CriticalBegin);
    Mock_Reset(hal_CriticalEnd);
    Mock_Reset(os_Fail);
    Mock_Reset(os_SubAlloc);
    Mock_Reset(test_Action);
    Mock_Reset(test_Action2);

    static os_subscription_t *seq[] = {&test_subscription, &test_subscription2 };
    Mock_ReturnsSequence(os_SubAlloc, 2, seq);

    os_subscriptions = NULL;

    test_subscription.topic = 0;
    test_subscription.action = NULL;
    test_subscription.next = &test_subscription;

    test_subscription2.topic = 0;
    test_subscription2.action = NULL;
    test_subscription2.next = &test_subscription2;
}

static void test_FirstSubscription(void) {
    setUp();

    os_Subscribe(TEST_TOPIC1, &test_Action);

    Assert_Equals(&test_subscription, os_subscriptions);
    Assert_Equals(NULL, test_subscription.next);
    Assert_Equals(TEST_TOPIC1, test_subscription.topic);
    Assert_Equals(&test_Action, test_subscription.action);
}

static void test_SecondSubscription_DifferentTopic(void) {
    setUp();

    os_Subscribe(TEST_TOPIC1, &test_Action);
    os_Subscribe(TEST_TOPIC2, &test_Action2);

    Assert_Equals(&test_subscription2, os_subscriptions);
    Assert_Equals(&test_subscription, test_subscription2.next);
    Assert_Equals(NULL, test_subscription.next);

    Assert_Equals(TEST_TOPIC1, test_subscription.topic);
    Assert_Equals(&test_Action, test_subscription.action);

    Assert_Equals(TEST_TOPIC2, test_subscription2.topic);
    Assert_Equals(&test_Action2, test_subscription2.action);
}

static void test_SecondSubscription_SameTopic_DifferentAction(void) {
    setUp();

    os_Subscribe(TEST_TOPIC1, &test_Action);
    os_Subscribe(TEST_TOPIC1, &test_Action2);

    Assert_Equals(&test_subscription2, os_subscriptions);
    Assert_Equals(&test_subscription, test_subscription2.next);
    Assert_Equals(NULL, test_subscription.next);

    Assert_Equals(TEST_TOPIC1, test_subscription.topic);
    Assert_Equals(&test_Action, test_subscription.action);

    Assert_Equals(TEST_TOPIC1, test_subscription2.topic);
    Assert_Equals(&test_Action2, test_subscription2.action);
}

static void test_DuplicateSubscriptionIgnored(void) {
    setUp();

    os_Subscribe(TEST_TOPIC1, &test_Action);
    os_Subscribe(TEST_TOPIC1, &test_Action);

    Assert_CallCount(1, os_SubAlloc);
    Assert_Equals(&test_subscription, os_subscriptions);
    Assert_Equals(NULL, test_subscription.next);
}

static void test_AllocationFailure(void) {
    setUp();
    Mock_Returns(os_SubAlloc, NULL);

    os_Subscribe(TEST_TOPIC1, &test_Action);

    Assert_CalledOnce(os_Fail);
    Assert_Called1(os_Fail, OS_FAIL_SUBSCRIBE_ALLOCATION);
}

static void test_WrapsInCriticalSection(void) {
    setUp();

    os_Subscribe(TEST_TOPIC1, &test_Action);

    Assert_CalledOnce(hal_CriticalBegin);
    Assert_CalledOnce(hal_CriticalEnd);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_FirstSubscription();
    test_SecondSubscription_DifferentTopic();
    test_SecondSubscription_SameTopic_DifferentAction();
    test_DuplicateSubscriptionIgnored();
    test_AllocationFailure();
    test_WrapsInCriticalSection();

    Assert_Save();
    return 0;
}
