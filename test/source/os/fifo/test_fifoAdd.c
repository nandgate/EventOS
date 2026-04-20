#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

#define TEST_UNSET          0x42

os_entry_t test_entry1;
os_entry_t test_entry2;
os_actionFifo_t os_actionFifo;

static void setUp(void)
{
    Test_Init();
    test_entry1.next = (os_entry_t *) TEST_UNSET;
    test_entry2.next = (os_entry_t *) TEST_UNSET;
}

static void test_AddToEmptyFifo(void)
{
    setUp();
    os_actionFifo.os_fifoHead = NULL;
    os_actionFifo.os_fifoTail = NULL;

    os_FifoAdd(&test_entry1);

    Assert_Equals(&test_entry1, os_actionFifo.os_fifoHead);
    Assert_Equals(&test_entry1, os_actionFifo.os_fifoTail);
    Assert_Equals(NULL, test_entry1.next);
}

static void test_AddToFifo(void)
{
    setUp();
    os_actionFifo.os_fifoHead = &test_entry1;
    os_actionFifo.os_fifoTail = &test_entry1;

    os_FifoAdd(&test_entry2);

    Assert_Equals(&test_entry1, os_actionFifo.os_fifoHead);
    Assert_Equals(&test_entry2, os_actionFifo.os_fifoTail);
    Assert_Equals(&test_entry2, test_entry1.next);
    Assert_Equals(NULL, test_entry2.next);
}

static void test_AddNull(void)
{
    setUp();
    os_actionFifo.os_fifoHead = NULL;
    os_actionFifo.os_fifoTail = NULL;

    os_FifoAdd(NULL);

    Assert_Equals(NULL, os_actionFifo.os_fifoHead);
    Assert_Equals(NULL, os_actionFifo.os_fifoTail);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_AddToEmptyFifo();
    test_AddToFifo();
    test_AddNull();

    Assert_Save();
    return 0;
}
