#include "os/os_p.h"
#include "stdlib.h"

os_actionFifo_t os_actionFifo;
os_tqEntry_t *os_tQueue;
os_subscription_t *os_subscriptions;

void os_Init(uint32_t sysTicksPerOsTick)
{
    os_subscriptions = NULL;
    os_actionFifo.os_fifoHead = NULL;
    os_actionFifo.os_fifoTail = NULL;
    os_MemInit();
    os_TimerInit(ARM_SCS_BASE, sysTicksPerOsTick);
}

void os_NullAction(os_context_t context) {
    (void)context;
}