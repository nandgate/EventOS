/**
 * @file os_p.h
 * Private header for EventOS (NEVER include this in your app).
 */
#pragma once

#include "os/os.h"
#include <stddef.h>
#include <stdint.h>

#define CTX_SLOT_WORDS(bytes) (((bytes) + sizeof(uint32_t) - 1) / sizeof(uint32_t))

#ifndef OS_CTX_LARGE_COUNT
#define OS_CTX_LARGE_COUNT 4
#endif

#ifndef OS_CTX_LARGE_SLOT
#define OS_CTX_LARGE_SLOT  64
#endif

#ifndef OS_CTX_SMALL_COUNT
#define OS_CTX_SMALL_COUNT 4
#endif

#ifndef OS_CTX_SMALL_SLOT
#define OS_CTX_SMALL_SLOT  16
#endif

#ifndef OS_MAX_ENTRIES
#define OS_MAX_ENTRIES 32
#endif

#ifndef OS_MAX_SUBSCRIPTIONS
#define OS_MAX_SUBSCRIPTIONS 8
#endif

typedef struct {
    struct os_entry_s *os_fifoHead;
    struct os_entry_s *os_fifoTail;
} os_actionFifo_t;

typedef struct os_ctx_s
{
    struct os_ctx_s *next;
    uint8_t count;
    uint16_t size;
    uint32_t data[];
} os_ctx_t;

typedef struct os_entry_s
{
    struct os_entry_s *next;
    uint32_t ticks;
    uint32_t key;
    os_ctx_t *ctx;
    os_action_t action;
} os_entry_t;

typedef struct os_subscription_s {
    struct os_subscription_s *next;
    uint32_t topic;
    os_action_t action;
} os_subscription_t;

os_ctx_t *os_ContextAcquire(os_context_t context);
os_ctx_t *os_ContextNew(uint32_t size);
void os_ContextRelease(os_ctx_t *ctx);

os_ctx_t *os_CtxAlloc(uint32_t size);
void os_CtxAllocInit(void);
void os_CtxFree(os_ctx_t *ctx);

os_entry_t *os_EntryAlloc(void);
void os_EntryAllocInit(void);
void os_EntryFree(os_entry_t *entry);
uint32_t os_EntryHighWater(void);
uint32_t os_EntryInUse(void);

void os_FifoAdd(os_entry_t *fifoEntry);
void os_FifoCancel(uint32_t key);
os_entry_t *os_FifoRemove(void);

os_subscription_t *os_SubAlloc(void);
void os_SubAllocInit(void);
void os_SubFree(os_subscription_t *sub);
uint32_t os_SubHighWater(void);
uint32_t os_SubInUse(void);

void os_TimerAdd(os_entry_t *entry);
void os_TimerInit(void);
void os_TimerRemove(uint32_t key);
