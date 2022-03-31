#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

Mock_Void1(os_MemFree, void*);
Mock_Void1(test_Action1, os_context_t);
Mock_Void1(test_Action2, os_context_t);
Mock_Void1(test_Action1a, os_context_t);
Mock_Void1(test_ActionMismatch, os_context_t);

#define TEST_UNSET          0x42
#define TEST_TOPIC1         42
#define TEST_TOPIC2         24
#define TEST_TOPIC_MISMATCH 100

os_subscription_t *os_subscriptions;
os_subscription_t test_subscription;
os_subscription_t test_subscription2;

static void setUp(void) {
    Mock_Reset(os_MemFree);
    Mock_Reset(test_Action1);
    Mock_Reset(test_Action2);
    Mock_Reset(test_Action1a);
    Mock_Reset(test_ActionMismatch);

    os_subscriptions= NULL;
}

static void setUp_OneSubscriptions(void) {
    setUp();
    setUp();

    os_subscriptions = &test_subscription;
    memset(test_subscription.actions, 0, sizeof(test_subscription.actions));
    test_subscription.actions[0] = test_Action1;
    test_subscription.topic = TEST_TOPIC1;
    test_subscription.next = NULL;
}

static void setUp_TwoSubscriptions(void) {
    setUp_OneSubscriptions();

    test_subscription.next = &test_subscription2;
    memset(test_subscription2.actions, 0, sizeof(test_subscription.actions));
    test_subscription2.actions[0] = test_Action2;
    test_subscription2.topic = TEST_TOPIC2;
    test_subscription2.next = NULL;
}

static void test_NoSubscriptions(void) {
    setUp();

    os_UnsubscribeAll(TEST_TOPIC1);

    Assert_NotCalled(os_MemFree);
}

static void test_NoTopic(void) {
    setUp_OneSubscriptions();

    os_UnsubscribeAll(TEST_TOPIC_MISMATCH);

    Assert_NotCalled(os_MemFree);
}

static void test_OneTopic(void) {
    setUp_OneSubscriptions();

    os_UnsubscribeAll(TEST_TOPIC1);

    Assert_Equals(NULL, os_subscriptions);

    Assert_CalledOnce(os_MemFree);
    Assert_Called1(os_MemFree, &test_subscription);
}

static void test_FirstTopic(void) {
    setUp_TwoSubscriptions();

    os_UnsubscribeAll(TEST_TOPIC1);

    Assert_Equals(&test_subscription2, os_subscriptions);

    Assert_CalledOnce(os_MemFree);
    Assert_Called1(os_MemFree, &test_subscription);
}

static void test_LastTopic(void) {
    setUp_TwoSubscriptions();

    os_UnsubscribeAll(TEST_TOPIC2);

    Assert_Equals(&test_subscription, os_subscriptions);
    Assert_Equals(NULL, test_subscription.next);

    Assert_CalledOnce(os_MemFree);
    Assert_Called1(os_MemFree, &test_subscription2);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_NoSubscriptions();
    test_NoTopic();
    test_OneTopic();
    test_FirstTopic();
    test_LastTopic();

    Assert_Save();
    return 0;
}