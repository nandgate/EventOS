/**
 * @file os.h
 * Private header for EventOS (NEVER include this in your app).
 */

#ifndef _OS_P_H_
#define _OS_P_H_

#include "os/arm_p.h"
#include "os/os.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct
{
    uint8_t count;
    uint16_t size;
    uint32_t data[];    // Use an uint32_t for alignment here, actual size depends on use
} os_ctx_t;

typedef struct os_actionEntry_s
{
    struct os_actionEntry_s *next;
    os_ctx_t *ctx;
    os_action_t action;
} os_actionEntry_t;

typedef struct
{
    os_actionEntry_t* os_fifoHead;
    os_actionEntry_t* os_fifoTail;
} os_actionFifo_t;

// Add to the tail of the list
void os_FifoAdd(os_actionEntry_t *fifoEntry);
// Remove from the head of the list
os_actionEntry_t *os_FifoRemove(void);

typedef struct os_tqEntry_s
{
    struct os_tqEntry_s *next;
    uint32_t ticks;
    os_action_t action;
    os_ctx_t *ctx;
} os_tqEntry_t;

#define OS_NUMBER_OF_SUBS 4

typedef struct os_subscription_s {
    struct os_subscription_s *next;     // Must be first field
    uint32_t topic;
    os_action_t actions[OS_NUMBER_OF_SUBS];
} os_subscription_t;

void os_TimerAdd(os_tqEntry_t *entry);
void os_TimerInit(arm_SCS_t *system, uint32_t sysTicksPerOsTick);
void SysTick_Handler(void);

os_ctx_t *os_ContextNew(uint32_t size);
void os_ContextUse(os_ctx_t *ctx);
os_ctx_t *os_ContextReuse(os_context_t context);
os_ctx_t *os_GetCtx(os_context_t context);

// Wrappers, for now...
void os_MemInit(void);
void *os_MemAlloc(uint32_t size);
void os_MemFree(void *mem);

#endif
