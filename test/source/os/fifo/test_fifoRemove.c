#include "os/os_p.h"
#include "UnitTest.h"

Mock_Vars(3);

os_entry_t test_entry1;
os_entry_t test_entry2;
os_actionFifo_t os_actionFifo;

static void setUp(void)
{
    Test_Init();
}

static void test_RemoveFromFifo(void)
{
    setUp();
    os_actionFifo.os_fifoHead = &test_entry1;
    os_actionFifo.os_fifoTail = &test_entry2;
    test_entry1.next = &test_entry2;
    test_entry2.next = NULL;

    os_entry_t *result = os_FifoRemove();

    Assert_Equals(&test_entry1, result);
    Assert_Equals(&test_entry2, os_actionFifo.os_fifoHead);
    Assert_Equals(&test_entry2, os_actionFifo.os_fifoTail);
    Assert_Equals(NULL, test_entry2.next);
}

static void test_RemoveLast(void)
{
    setUp();
    os_actionFifo.os_fifoHead = &test_entry1;
    os_actionFifo.os_fifoTail = &test_entry1;
    test_entry1.next = NULL;

    os_entry_t *result = os_FifoRemove();

    Assert_Equals(&test_entry1, result);
    Assert_Equals(NULL, os_actionFifo.os_fifoHead);
    Assert_Equals(NULL, os_actionFifo.os_fifoTail);
}

static void test_RemoveEmpty(void)
{
    setUp();
    os_actionFifo.os_fifoHead = NULL;
    os_actionFifo.os_fifoTail = NULL;

    os_entry_t *result = os_FifoRemove();

    Assert_Equals(NULL, result);
}

int main(int argc, char **argv)
{
    Assert_Init();

    test_RemoveFromFifo();
    test_RemoveLast();
    test_RemoveEmpty();

    Assert_Save();
    return 0;
}
