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

static void setUpOneTopic(char *name) {
    setUp();

    //printf("%s\n", name);
    os_subscriptions = &test_subscription;

    test_subscription.topic = TEST_TOPIC1;
    memset(test_subscription.actions, 0, sizeof(test_subscription.actions));
    test_subscription.actions[0] = test_Action1;
    test_subscription.actions[1] = NULL;
    test_subscription.next = NULL;
}

static void setUpTwoTopics(char *name)
{
    setUpOneTopic(name);

    test_subscription.next = &test_subscription2;
    test_subscription2.topic = TEST_TOPIC2;
    memset(test_subscription2.actions, 0, sizeof(test_subscription.actions));
    test_subscription2.actions[0] = test_Action2;
    test_subscription2.actions[1] = NULL;
    test_subscription2.next = NULL;
}

static void test_NoSubscriptions(void) {
    setUp();

    os_Unsubscribe(TEST_TOPIC1, test_Action1);

    Assert_NotCalled(os_MemFree);
}

static void test_OneTopic_ActionMismatch(void) {
    setUpOneTopic("test_OneTopic_ActionMismatch");

    os_Unsubscribe(TEST_TOPIC1, test_ActionMismatch);

    Assert_Equals(&test_subscription, os_subscriptions);
    Assert_Equals(test_Action1, test_subscription.actions[0]);
    Assert_NotCalled(os_MemFree);
}

static void test_OneTopic_TopicMismatch(void) {
    setUpOneTopic("test_OneTopic_TopicMismatch");

    os_Unsubscribe(TEST_TOPIC_MISMATCH, test_Action1);

    Assert_Equals(&test_subscription, os_subscriptions);
    Assert_Equals(test_Action1, test_subscription.actions[0]);
    Assert_NotCalled(os_MemFree);
}

static void test_OneTopic_OneSubscriber(void) {
    setUpOneTopic("test_OneTopic_OneSubscriber");

    os_Unsubscribe(TEST_TOPIC1, test_Action1);

    Assert_Equals(NULL, os_subscriptions);

    Assert_CalledOnce(os_MemFree);
    Assert_Called1(os_MemFree, &test_subscription);
}

static void test_OneTopic_TwoSubscriber_First(void) {
    setUpOneTopic("test_OneTopic_TwoSubscriber_First");
    os_subscriptions->actions[1] = &test_Action1a;
    os_subscriptions->actions[2] = NULL;

    os_Unsubscribe(TEST_TOPIC1, test_Action1);

    Assert_Equals(&test_subscription, os_subscriptions);
    Assert_Equals(&test_Action1a, test_subscription.actions[0]);
    Assert_Equals(NULL, test_subscription.actions[1]);

    Assert_NotCalled(os_MemFree);
}

static void test_OneTopic_TwoSubscriber_Second(void) {
    setUpOneTopic("test_OneTopic_TwoSubscriber_Second");
    os_subscriptions->actions[1] = &test_Action1a;
    os_subscriptions->actions[2] = NULL;

    os_Unsubscribe(TEST_TOPIC1, test_Action1a);

    Assert_Equals(&test_subscription, os_subscriptions);
    Assert_Equals(&test_Action1, test_subscription.actions[0]);
    Assert_Equals(NULL, test_subscription.actions[1]);

    Assert_NotCalled(os_MemFree);
}

static void test_OneTopic_MaxSubscribers(void) {
    setUpOneTopic("test_OneTopic_MaxSubscribers");
    for(int i= 1; i< OS_NUMBER_OF_SUBS; i++) {
        os_subscriptions->actions[i] = &test_Action1a;
    }

    os_Unsubscribe(TEST_TOPIC1, test_Action1);

    Assert_Equals(&test_subscription, os_subscriptions);
    for(int i= 1; i< OS_NUMBER_OF_SUBS-1; i++) {
        Assert_Equals(&test_Action1a, os_subscriptions->actions[i]);
    }
    Assert_Equals(NULL, test_subscription.actions[OS_NUMBER_OF_SUBS-1]);

    Assert_NotCalled(os_MemFree);
}

static void test_TwoTopic_ActionMismatch(void) {
    setUpTwoTopics("test_TwoTopic_ActionMismatch");

    os_Unsubscribe(TEST_TOPIC2, test_ActionMismatch);

    Assert_Equals(&test_subscription, os_subscriptions);
    Assert_Equals(test_Action1, test_subscription.actions[0]);
    Assert_Equals(NULL, test_subscription.actions[2]);

    Assert_Equals(&test_subscription2, os_subscriptions->next);
    Assert_Equals(test_Action2, test_subscription2.actions[0]);
    Assert_Equals(NULL, test_subscription2.actions[1]);

    Assert_NotCalled(os_MemFree);
}

static void test_TwoTopic_TopicMismatch(void) {
    setUpTwoTopics("test_TwoTopic_TopicMismatch");

    os_Unsubscribe(TEST_TOPIC_MISMATCH, test_Action2);

    Assert_Equals(&test_subscription, os_subscriptions);
    Assert_Equals(test_Action1, test_subscription.actions[0]);
    Assert_Equals(NULL, test_subscription.actions[2]);

    Assert_Equals(&test_subscription2, os_subscriptions->next);
    Assert_Equals(test_Action2, test_subscription2.actions[0]);
    Assert_Equals(NULL, test_subscription2.actions[1]);

    Assert_NotCalled(os_MemFree);
}

static void test_TwoTopic_OneSubscriber(void) {
    setUpTwoTopics("test_TwoTopic_OneSubscriber");

    os_Unsubscribe(TEST_TOPIC2, test_Action2);

    Assert_Equals(NULL, os_subscriptions->next);

    Assert_CalledOnce(os_MemFree);
    Assert_Called1(os_MemFree, &test_subscription2);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_NoSubscriptions();

    test_OneTopic_ActionMismatch();
    test_OneTopic_TopicMismatch();
    test_OneTopic_OneSubscriber();
    test_OneTopic_TwoSubscriber_First();
    test_OneTopic_TwoSubscriber_Second();
    test_OneTopic_MaxSubscribers();

    test_TwoTopic_ActionMismatch();
    test_TwoTopic_TopicMismatch();
    test_TwoTopic_OneSubscriber();

    Assert_Save();
    return 0;
}