#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

Mock_Value1(void *, os_MemAlloc, uint32_t);
Mock_Void1(test_Action, os_context_t);
Mock_Void1(test_Action2, os_context_t);

#define TEST_UNSET          0x42
#define TEST_TOPIC1          42
#define TEST_TOPIC2         24

os_subscription_t *os_subscriptions;
os_subscription_t test_subscription;
os_subscription_t test_subscription2;

static void setUp(void)
{
    Mock_Reset(os_MemAlloc);
    Mock_Reset(test_Action);
    Mock_Reset(test_Action2);

    static void *seq[] = {&test_subscription, &test_subscription2 };
    Mock_ReturnsSequence(os_MemAlloc, 2, seq);

    os_subscriptions = NULL;

    test_subscription.topic = TEST_UNSET;
    memset(test_subscription.actions, TEST_UNSET, sizeof(test_subscription.actions));
    test_subscription.next = &test_subscription;

    test_subscription2.topic = TEST_UNSET;
    memset(test_subscription2.actions, TEST_UNSET, sizeof(test_subscription.actions));
    test_subscription2.next = &test_subscription2;
}

static void test_FirstSubscription(void) {
    setUp();

    os_Subscribe(TEST_TOPIC1, &test_Action);

    Assert_Equals(&test_subscription, os_subscriptions);
    Assert_Equals(NULL, test_subscription.next);

    Assert_Equals(TEST_TOPIC1, test_subscription.topic);
    Assert_Equals(&test_Action, test_subscription.actions[0]);
    for(int i= 1; i < OS_NUMBER_OF_SUBS; i++) {
        Assert_Equals(NULL, test_subscription.actions[i]);
    }
}

static void test_SecondSubscription_TwoTopics(void) {
    setUp();

    os_Subscribe(TEST_TOPIC1, &test_Action);
    os_Subscribe(TEST_TOPIC2, &test_Action2);

    Assert_Equals(&test_subscription2, os_subscriptions);
    Assert_Equals(&test_subscription, test_subscription2.next);
    Assert_Equals(NULL, test_subscription.next);

    Assert_Equals(TEST_TOPIC1, test_subscription.topic);
    Assert_Equals(&test_Action, test_subscription.actions[0]);

    Assert_Equals(TEST_TOPIC2, test_subscription2.topic);
    Assert_Equals(&test_Action2, test_subscription2.actions[0]);
}

static void test_SecondSubscription_SameTopic(void) {
    setUp();

    os_Subscribe(TEST_TOPIC1, &test_Action);
    os_Subscribe(TEST_TOPIC1, &test_Action2);

    Assert_Equals(&test_subscription, os_subscriptions);
    Assert_Equals(NULL, test_subscription.next);

    Assert_Equals(TEST_TOPIC1, test_subscription.topic);
    Assert_Equals(&test_Action, test_subscription.actions[0]);
    Assert_Equals(&test_Action2, test_subscription.actions[1]);
}

static void test_ThirdSubscription_ExsistingTopic(void) {
    setUp();

    os_Subscribe(TEST_TOPIC1, &test_Action);
    os_Subscribe(TEST_TOPIC2, &test_Action2);
    os_Subscribe(TEST_TOPIC1, &test_Action2);

    Assert_Equals(&test_subscription2, os_subscriptions);
    Assert_Equals(&test_subscription, test_subscription2.next);
    Assert_Equals(NULL, test_subscription.next);

    Assert_Equals(TEST_TOPIC1, test_subscription.topic);
    Assert_Equals(&test_Action, test_subscription.actions[0]);
    Assert_Equals(&test_Action2, test_subscription.actions[1]);

    Assert_Equals(TEST_TOPIC2, test_subscription2.topic);
    Assert_Equals(&test_Action2, test_subscription2.actions[0]);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_FirstSubscription();
    test_SecondSubscription_TwoTopics();
    test_SecondSubscription_SameTopic();
    test_ThirdSubscription_ExsistingTopic();

    Assert_Save();
    return 0;
}