/**
 * @file os.h
 * Public header for EventOS (include this in your app).
 */

/**
 * @page Overview EventOS Overview
 *
 * For the design tour — concepts, patterns, the concurrency model, and porting notes —
 * see [EventOS.md](../../EventOS.md) in the repository root. This reference documents the
 * public API only.
 *
 * @section License License
 *
 * EventOS is published under the [BSD Zero Clause License (0BSD)](https://opensource.org/license/0bsd).
 *
 * &copy;2022-2026, NAND Gate Technologies, LLC.
 */

#pragma once

#ifdef OS_USE_CONFIG_H
#include "os_config.h"
#endif

#include <stdint.h>

/**
 * EventOS release version (Semantic Versioning).
 */
#define OS_VERSION "2.0.0"

#ifndef OS_TICKS_PER_SECOND
#define OS_TICKS_PER_SECOND 1000
#endif

/**
 * A Context is a block of memory holding application specific state, but managed by the OS.
 *
 * Applications must not cache context pointers. Contexts are allocated and freed exclusively by the
 * OS; applications should not retain a context reference past the current action's return, except
 * by handing it to another action via the OS-aware functions (os_DoWith, os_DoAfterWith,
 * os_PublishWith, os_PutWith). The OS is free to relocate a context between invocations, so a
 * cached pointer may not refer to the same memory later. The OS will never manipulate or use the
 * data in a context.
 */
typedef void *os_context_t;

/**
 * Actions are units of work to be performed by the applications.
 *
 * Actions typically have contexts associated with them
 * that carry state through the system. The OS schedules the execution of actions, and manages the contexts that are
 * associated with them.
 * 1. Actions are run to completion.
 * 2. Actions are cooperative (non-preemptive).
 * 3. Actions must not block. Long-running work is broken into steps that re-enqueue themselves.
 */
typedef void(*os_action_t)(os_context_t context);

/**
 * Use this as the contextSize argument to os_Do, os_DoAfter, or os_Publish when the action
 * doesn't need context data. No context is allocated and the action receives a NULL pointer.
 */
#define OS_NO_CONTEXT 0

/**
 * Use this when a key is not needed. Entries with OS_NO_KEY are never matched by os_CancelPending().
 */
#define OS_NO_KEY 0

/**
 * A no-op action. Pass this anywhere the API requires an action pointer but no work should be
 * performed (e.g. as the timeout action of an os_Put/os_PutWith posting that doesn't need a
 * timeout notification). EventOS APIs do not accept NULL as an action pointer.
 */
void os_NullAction(os_context_t context);

/**
 * Failure reasons passed to os_Fail().
 */
typedef enum {
    OS_FAIL_CONTEXT_ALLOCATION,        /**< os_ContextNew could not allocate a context. */
    OS_FAIL_DO_ALLOCATION,             /**< os_Do could not allocate a queue entry. */
    OS_FAIL_DO_AFTER_ALLOCATION,       /**< os_DoAfter could not allocate a queue entry. */
    OS_FAIL_DO_AFTER_WITH_ALLOCATION,  /**< os_DoAfterWith could not allocate a queue entry. */
    OS_FAIL_DO_WITH_ALLOCATION,        /**< os_DoWith could not allocate a queue entry. */
    OS_FAIL_ENTRY_FREE,                /**< Null pointer passed to entry free. */
    OS_FAIL_PUT_ALLOCATION,            /**< os_Put could not allocate a bulletin board entry. */
    OS_FAIL_PUT_WITH_ALLOCATION,       /**< os_PutWith could not allocate a bulletin board entry. */
    OS_FAIL_SUB_FREE,                  /**< Null pointer passed to subscription free. */
    OS_FAIL_SUBSCRIBE_ALLOCATION,      /**< os_Subscribe could not allocate a subscription. */
} os_fail_t;

/**
 * User-overridable failure handler. Called when the OS encounters an unrecoverable error. Each
 * shipped platform port provides a weak default (halt on bluepill, exit on POSIX); override it to
 * install your own policy. If this function returns, the calling OS function will abort its
 * operation and return without completing. No further OS guarantees are made after os_Fail is
 * called.
 * @param reason The reason for the failure
 */
void os_Fail(os_fail_t reason);

/**
 * User-overridable hook invoked by os_Exec() immediately before dispatching an action. Intended
 * for instrumentation (per-action runtime measurement, tracing, coverage). The default is a
 * weak no-op shipped by each platform port; override it to install measurement.
 * @param action The action about to be invoked.
 */
void os_ActionBegin(os_action_t action);

/**
 * User-overridable hook invoked by os_Exec() immediately after an action returns. Pairs with
 * os_ActionBegin() to bracket the action call — useful for stopping a timer started in
 * os_ActionBegin and recording the elapsed time. The default is a weak no-op shipped by each
 * platform port; override it to install measurement.
 * @param action The action that just returned.
 */
void os_ActionEnd(os_action_t action);

/**
 * Current count of live entries across the FIFO and timer queue. Primary load metric — see
 * EventOS.md §2.6. Safe to call from any action.
 */
uint32_t os_EntryInUse(void);

/**
 * Running maximum of os_EntryInUse() since os_Init(). Sample periodically and log to track
 * peak load and alert as it approaches OS_MAX_ENTRIES.
 */
uint32_t os_EntryHighWater(void);

/** @defgroup Administrative EventOS Administration
 *  Functions for administrating EventOS. These functions are typically called from main().
 *  @{
 */

/**
 * Function to initialize EventOS. Clears all pools (entry, subscription, context) and
 * queues (FIFO, timer queue) to their startup state.
 *
 * This function should be called once before any other calls to the OS are made.
 */
void os_Init(void);

/**
 * Execute one pending action from the FIFO. Pops the head entry, invokes the action, releases the
 * context's reference on return, and frees the entry. Returns immediately with no work done if the
 * FIFO is empty.
 *
 * This function is typically called within a 'while' loop in main().
 */
void os_Exec(void);

/**
 * Advance the timer queue by one tick.
 *
 * This function must be called by the platform's tick source at the desired tick rate — a periodic
 * timer interrupt on bare-metal targets, a cooperative polling loop on POSIX. Expired entries are
 * moved to the FIFO for execution.
 */
void os_Tick(void);

/** @} */

/** @defgroup DoAction Do actions (ASAP).
 *  Functions that schedule an action on the FIFO for execution as soon as the scheduler reaches it.
 *  @{
 */

/**
 * Function to enqueue an action to be performed using a new context.
 * @param action The action to be performed (callback)
 * @param contextSize The size of the context data required by the action
 * @return Pointer to the context data that will be passed to the action when it is invoked
 */
os_context_t os_Do(os_action_t action, uint32_t contextSize);

/**
 * Function to enqueue an action to be performed using an existing context.
 * @param action The action to be performed (callback)
 * @param context Pointer to the context data that will be passed to the action when it is invoked
 */
void os_DoWith(os_action_t action, os_context_t context);

/** @} */

/** @defgroup DoAfter Do actions on a schedule (Timers).
 *  Functions that schedule an action on the timer queue for delayed execution after a specified
 *  number of ticks.
 *  @{
 */

/**
 * Function to enqueue an action to be performed using a new context after the specified number of ticks have
 * elapsed.
 * @param action The action to be performed (callback)
 * @param contextSize The size of the context data required by the action
 * @param key Caller-supplied key for cancellation via os_CancelPending(); use OS_NO_KEY if not cancellable
 * @param ticks The number of ticks to wait until the action is enqueued
 * @return Pointer to the context data that will be passed to the action when it is invoked
 */
os_context_t os_DoAfter(os_action_t action, uint32_t contextSize, uint32_t key, uint32_t ticks);

/**
 * Function to enqueue an action to be performed using an existing context after the specified number of ticks have
 * elapsed.
 * @param action The action to be performed (callback)
 * @param context Pointer to the context data that will be passed to the action when it is invoked
 * @param key Caller-supplied key for cancellation via os_CancelPending(); use OS_NO_KEY if not cancellable
 * @param ticks The number of ticks to wait until the action is enqueued
 */
void os_DoAfterWith(os_action_t action, os_context_t context, uint32_t key, uint32_t ticks);

/**
 * Function to cancel all pending actions matching the specified key, whether they are waiting on
 * the timer queue or already promoted to the FIFO. Entries with OS_NO_KEY are never cancelled.
 * @param key The key identifying which pending actions to cancel
 */
void os_CancelPending(uint32_t key);

/** @} */

/**
 * @defgroup Publish-Subscribe Do actions using publish/subscribe (Topics).
 * Functions for executing actions using the publish/subscribe design pattern.
 * @{
 */

/**
 * Function to subscribe to a topic.
 * *Note:* A duplicate subscription is silently ignored — an action cannot subscribe to the same
 * topic more than once.
 * @param topic The topic to subscribe to.
 * @param action The action to invoke when the topic is published.
 */
void os_Subscribe(uint32_t topic, os_action_t action);

/**
 * Function to unsubscribe from a topic.
 * *Note:* Unsubscribing an action from a topic it is not subscribed to is ignored.
 * @param topic The topic to unsubscribe from.
 * @param action The action to unsubscribe from
 */
void os_Unsubscribe(uint32_t topic, os_action_t action);

/**
 * Function to unsubscribe all actions from a topic.
 * *Note:* If the topic has no subscribers, this call is a no-op.
 * @param topic The topic to remove all subscriptions
 */
void os_UnsubscribeAll(uint32_t topic);

/**
 * Function to publish to a topic using a new context. A context is always allocated, even if the
 * topic has no subscribers — in that case the context is dispatched to os_NullAction so the
 * returned pointer is still valid for the caller to fill in.
 * @param topic The topic to publish
 * @param contextSize The size of the context data required by the action
 * @return Pointer to the context data that will be passed to all subscribers.
 */
os_context_t os_Publish(uint32_t topic, uint32_t contextSize);

/**
 * Function to publish to a topic using an existing context.
 * @param topic The topic to publish
 * @param context Pointer to the context data that will be passed to all subscribers.
 */
void os_PublishWith(uint32_t topic, os_context_t context);

/** @} */

/** @defgroup BulletinBoard Keyed context store (Bulletin Board).
 *  Functions for posting a context under a key with a TTL, and retrieving it before it expires.
 *  @{
 */

/**
 * Function to allocate a new context and post it to the bulletin board. The bulletin board owns the context.
 * The key shares the same namespace as os_DoAfter/os_CancelPending keys.
 * @param key The key to access the item on the bulletin board
 * @param contextSize The size of the context data to allocate
 * @param timeoutAction Action to invoke with the context if it expires before being retrieved
 * @param ticks Number of ticks the context can remain on the bulletin board before expiring
 * @return Pointer to the context data the caller should fill in
 */
os_context_t os_Put(uint32_t key, uint32_t contextSize, os_action_t timeoutAction, uint32_t ticks);

/**
 * Function to post an existing context to the bulletin board. The bulletin board acquires its own
 * reference; the caller's action continues to hold a valid reference for the rest of its execution.
 * The key shares the same namespace as os_DoAfter/os_CancelPending keys.
 * @param key The key to access the item on the bulletin board
 * @param context Pointer to the context to be stored on the bulletin board
 * @param timeoutAction Action to invoke with the context if it expires before being retrieved
 * @param ticks Number of ticks the context can remain on the bulletin board before expiring
 */
void os_PutWith(uint32_t key, os_context_t context, os_action_t timeoutAction, uint32_t ticks);

/**
 * Function to retrieve and remove a context from the bulletin board. The calling action takes ownership:
 * it may read the data immediately and then drop the pointer (the OS frees the context automatically
 * when the action returns), or forward it via any OS-aware function (os_DoWith, os_DoAfterWith,
 * os_PublishWith, os_PutWith) to keep the context alive. The pending timeout is cancelled.
 *
 * An action may hold multiple contexts simultaneously via repeated os_Get calls under different keys;
 * every held context is released automatically when the action returns.
 *
 * @param key The key to the item on the bulletin board
 * @return Pointer to the context, or NULL if the key is not on the bulletin board
 */
os_context_t os_Get(uint32_t key);

/** @} */
