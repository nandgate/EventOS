#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

Mock_Void(hal_CriticalBegin);
Mock_Void(hal_CriticalEnd);
Mock_Void1(os_SubFree, os_subscription_t *);
Mock_Void1(test_Action1, os_context_t);
Mock_Void1(test_Action2, os_context_t);
Mock_Void1(test_Action3, os_context_t);

#define TEST_TOPIC1         42
#define TEST_TOPIC2         24
#define TEST_TOPIC_MISMATCH 100

os_subscription_t *os_subscriptions;
os_subscription_t test_subscription;
os_subscription_t test_subscription2;
os_subscription_t test_subscription3;

static void setUp(void) {
    Test_Init();
    Mock_Reset(hal_CriticalBegin);
    Mock_Reset(hal_CriticalEnd);
    Mock_Reset(os_SubFree);
    Mock_Reset(test_Action1);
    Mock_Reset(test_Action2);
    Mock_Reset(test_Action3);

    os_subscriptions = NULL;
}

static void setUp_OneSub(void) {
    setUp();

    os_subscriptions = &test_subscription;
    test_subscription.topic = TEST_TOPIC1;
    test_subscription.action = test_Action1;
    test_subscription.next = NULL;
}

static void setUp_TwoSubs_DifferentTopics(void) {
    setUp_OneSub();

    test_subscription.next = &test_subscription2;
    test_subscription2.topic = TEST_TOPIC2;
    test_subscription2.action = test_Action2;
    test_subscription2.next = NULL;
}

static void setUp_TwoSubs_SameTopic(void) {
    setUp();

    os_subscriptions = &test_subscription;
    test_subscription.topic = TEST_TOPIC1;
    test_subscription.action = test_Action1;
    test_subscription.next = &test_subscription2;

    test_subscription2.topic = TEST_TOPIC1;
    test_subscription2.action = test_Action2;
    test_subscription2.next = NULL;
}

static void test_NoSubscriptions(void) {
    setUp();

    os_UnsubscribeAll(TEST_TOPIC1);

    Assert_NotCalled(os_SubFree);
}

static void test_NoTopicMatch(void) {
    setUp_OneSub();

    os_UnsubscribeAll(TEST_TOPIC_MISMATCH);

    Assert_Equals(&test_subscription, os_subscriptions);
    Assert_NotCalled(os_SubFree);
}

static void test_OneSub_Match(void) {
    setUp_OneSub();

    os_UnsubscribeAll(TEST_TOPIC1);

    Assert_IsNull(os_subscriptions);
    Assert_CalledOnce(os_SubFree);
    Assert_Called1(os_SubFree, &test_subscription);
}

static void test_TwoSubs_DifferentTopics_RemoveFirst(void) {
    setUp_TwoSubs_DifferentTopics();

    os_UnsubscribeAll(TEST_TOPIC1);

    Assert_Equals(&test_subscription2, os_subscriptions);
    Assert_CalledOnce(os_SubFree);
    Assert_Called1(os_SubFree, &test_subscription);
}

static void test_TwoSubs_DifferentTopics_RemoveLast(void) {
    setUp_TwoSubs_DifferentTopics();

    os_UnsubscribeAll(TEST_TOPIC2);

    Assert_Equals(&test_subscription, os_subscriptions);
    Assert_Equals(NULL, test_subscription.next);
    Assert_CalledOnce(os_SubFree);
    Assert_Called1(os_SubFree, &test_subscription2);
}

static void test_TwoSubs_SameTopic_RemovesAll(void) {
    setUp_TwoSubs_SameTopic();

    os_UnsubscribeAll(TEST_TOPIC1);

    Assert_IsNull(os_subscriptions);
    Assert_CallCount(2, os_SubFree);
}

static void test_WrapsInCriticalSection(void) {
    setUp_OneSub();

    os_UnsubscribeAll(TEST_TOPIC1);

    Assert_CalledOnce(hal_CriticalBegin);
    Assert_CalledOnce(hal_CriticalEnd);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_NoSubscriptions();
    test_NoTopicMatch();
    test_OneSub_Match();
    test_TwoSubs_DifferentTopics_RemoveFirst();
    test_TwoSubs_DifferentTopics_RemoveLast();
    test_TwoSubs_SameTopic_RemovesAll();
    test_WrapsInCriticalSection();

    Assert_Save();
    return 0;
}
