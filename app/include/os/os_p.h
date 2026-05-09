/**
 * @file os_p.h
 * Private header for EventOS (NEVER include this in your app).
 */
#pragma once

#include "os/os.h"
#include <stddef.h>
#include <stdint.h>

typedef struct os_ctx_s
{
    struct os_ctx_s *next;  // free-list (while free); held-list (while held by an action)
    uint8_t count;
    uint16_t size;
    uint32_t data[];    // Use an uint32_t for alignment here, actual size depends on use
} os_ctx_t;

typedef struct os_entry_s
{
    struct os_entry_s *next;
    uint32_t ticks;     // 0/unused while on FIFO; delta-encoded ticks while on timer queue
    uint32_t key;
    os_ctx_t *ctx;
    os_action_t action;
} os_entry_t;

typedef struct
{
    os_entry_t* os_fifoHead;
    os_entry_t* os_fifoTail;
} os_actionFifo_t;

// Add to the tail of the list
void os_FifoAdd(os_entry_t *fifoEntry);
void os_FifoCancel(uint32_t key);
// Remove from the head of the list
os_entry_t *os_FifoRemove(void);

typedef struct os_subscription_s {
    struct os_subscription_s *next;     // Must be first field
    uint32_t topic;
    os_action_t action;
} os_subscription_t;

void os_TimerAdd(os_entry_t *entry);
void os_TimerInit(void);
void os_TimerRemove(uint32_t key);

os_ctx_t *os_ContextNew(uint32_t size);
void os_ContextRelease(os_ctx_t *ctx);
os_ctx_t *os_ContextAcquire(os_context_t context);



// Entry pool — fixed-size slots for FIFO/timer queue entries.
#ifndef OS_MAX_ENTRIES
#define OS_MAX_ENTRIES 32
#endif

void os_EntryAllocInit(void);
os_entry_t *os_EntryAlloc(void);
void os_EntryFree(os_entry_t *entry);
uint32_t os_EntryInUse(void);
uint32_t os_EntryHighWater(void);

// Subscription pool — fixed-size slots for pub/sub subscriptions.
#ifndef OS_MAX_SUBSCRIPTIONS
#define OS_MAX_SUBSCRIPTIONS 8
#endif

void os_SubAllocInit(void);
os_subscription_t *os_SubAlloc(void);
void os_SubFree(os_subscription_t *sub);
uint32_t os_SubInUse(void);
uint32_t os_SubHighWater(void);

// Context allocator seam — link-time pluggable.
// Provided by: ctxAllocMalloc.c (malloc/free), ctxAllocPool.c (two-bucket), or user.
#ifndef OS_CTX_SMALL_SLOT
#define OS_CTX_SMALL_SLOT  16
#endif
#ifndef OS_CTX_SMALL_COUNT
#define OS_CTX_SMALL_COUNT 4
#endif
#ifndef OS_CTX_LARGE_SLOT
#define OS_CTX_LARGE_SLOT  64
#endif
#ifndef OS_CTX_LARGE_COUNT
#define OS_CTX_LARGE_COUNT 4
#endif

#define CTX_SLOT_WORDS(bytes) (((bytes) + sizeof(uint32_t) - 1) / sizeof(uint32_t))

void os_CtxAllocInit(void);
os_ctx_t *os_CtxAlloc(uint32_t size);
void os_CtxFree(os_ctx_t *ctx);
