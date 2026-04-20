#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

Mock_Void(hal_CriticalBegin);
Mock_Void(hal_CriticalEnd);
Mock_Void1(os_SubFree, os_subscription_t *);
Mock_Void1(test_Action1, os_context_t);
Mock_Void1(test_Action2, os_context_t);
Mock_Void1(test_Action3, os_context_t);
Mock_Void1(test_ActionMismatch, os_context_t);

#define TEST_TOPIC1         42
#define TEST_TOPIC2         24
#define TEST_TOPIC3         12
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
    Mock_Reset(test_ActionMismatch);

    os_subscriptions = NULL;
}

static void setUpOneSub(void) {
    setUp();

    os_subscriptions = &test_subscription;
    test_subscription.topic = TEST_TOPIC1;
    test_subscription.action = test_Action1;
    test_subscription.next = NULL;
}

static void setUpThreeSubs(void) {
    setUp();

    os_subscriptions = &test_subscription;
    test_subscription.topic = TEST_TOPIC1;
    test_subscription.action = test_Action1;
    test_subscription.next = &test_subscription2;

    test_subscription2.topic = TEST_TOPIC2;
    test_subscription2.action = test_Action2;
    test_subscription2.next = &test_subscription3;

    test_subscription3.topic = TEST_TOPIC3;
    test_subscription3.action = test_Action3;
    test_subscription3.next = NULL;
}

static void test_NoSubscriptions(void) {
    setUp();

    os_Unsubscribe(TEST_TOPIC1, test_Action1);

    Assert_NotCalled(os_SubFree);
}

static void test_OneSub_NoMatch_TopicMismatch(void) {
    setUpOneSub();

    os_Unsubscribe(TEST_TOPIC_MISMATCH, test_Action1);

    Assert_Equals(&test_subscription, os_subscriptions);
    Assert_NotCalled(os_SubFree);
}

static void test_OneSub_NoMatch_ActionMismatch(void) {
    setUpOneSub();

    os_Unsubscribe(TEST_TOPIC1, test_ActionMismatch);

    Assert_Equals(&test_subscription, os_subscriptions);
    Assert_NotCalled(os_SubFree);
}

static void test_OneSub_Match_Removes(void) {
    setUpOneSub();

    os_Unsubscribe(TEST_TOPIC1, test_Action1);

    Assert_IsNull(os_subscriptions);
    Assert_CalledOnce(os_SubFree);
    Assert_Called1(os_SubFree, &test_subscription);
}

static void test_ThreeSubs_RemovesFirst(void) {
    setUpThreeSubs();

    os_Unsubscribe(TEST_TOPIC1, test_Action1);

    Assert_Equals(&test_subscription2, os_subscriptions);
    Assert_Equals(&test_subscription3, test_subscription2.next);
    Assert_CalledOnce(os_SubFree);
    Assert_Called1(os_SubFree, &test_subscription);
}

static void test_ThreeSubs_RemovesMiddle(void) {
    setUpThreeSubs();

    os_Unsubscribe(TEST_TOPIC2, test_Action2);

    Assert_Equals(&test_subscription, os_subscriptions);
    Assert_Equals(&test_subscription3, test_subscription.next);
    Assert_Equals(NULL, test_subscription3.next);
    Assert_CalledOnce(os_SubFree);
    Assert_Called1(os_SubFree, &test_subscription2);
}

static void test_ThreeSubs_RemovesLast(void) {
    setUpThreeSubs();

    os_Unsubscribe(TEST_TOPIC3, test_Action3);

    Assert_Equals(&test_subscription, os_subscriptions);
    Assert_Equals(&test_subscription2, test_subscription.next);
    Assert_Equals(NULL, test_subscription2.next);
    Assert_CalledOnce(os_SubFree);
    Assert_Called1(os_SubFree, &test_subscription3);
}

static void test_WrapsInCriticalSection(void) {
    setUpOneSub();

    os_Unsubscribe(TEST_TOPIC1, test_Action1);

    Assert_CalledOnce(hal_CriticalBegin);
    Assert_CalledOnce(hal_CriticalEnd);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_NoSubscriptions();
    test_OneSub_NoMatch_TopicMismatch();
    test_OneSub_NoMatch_ActionMismatch();
    test_OneSub_Match_Removes();
    test_ThreeSubs_RemovesFirst();
    test_ThreeSubs_RemovesMiddle();
    test_ThreeSubs_RemovesLast();
    test_WrapsInCriticalSection();

    Assert_Save();
    return 0;
}
