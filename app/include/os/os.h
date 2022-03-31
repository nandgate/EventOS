/**
 * @file os.h
 * Public header for EventOS (include this in your app).
 */

/**
 * @page Overview EventOS Overview
 * Exposition on the ideas and philosophy of EventOS goes here.
 *
 * @tableofcontents
 *
 * @section sec1 Section One
 * Blah blah blah
 *
 * @section Licence License
 *
 * EventOS is published under a the [MIT License.](https://mit-license.org)
 *
 * &copy;2022, NAND Gate Technologies, LLC.
 */

#ifndef _OS_H_
#define _OS_H_

#include <stdint.h>

/**
 * A Context is a block of memory holding application specific state, but managed by the OS.
 *
 * Contexts are allocated
 * and freed exclusively by the OS. Applications should never hold onto a context reference, or assume that the same
 * context (data) will reside at the same address in the future (e.g. the OS if free to relocate a context).  The OS
 * will never manipulate or use the data in a context.
 */
typedef void *os_context_t;

/**
 * Action are units of work to be performed by the applications.
 *
 * Action typically have contexts associated with them
 * that carry state through the system. The OS schedules the execution of actions, and manages the context that are
 * associated with them.
 * 1. Action are run to completion.
 * 2. Action are cooperative (non-preemptive)
 * 3. another important thing...
 */
typedef void(*os_action_t)(os_context_t context);

/**
 * Use this when a context size needs to be specified but the context isn't needed or used.
 */
#define OS_NO_CONTEXT 0

void os_NullAction(os_context_t context);

/** @defgroup Administrative EventOS Administration
 *  Functions for administrating EventOS. These function are typically called from main().
 *  @{
 */

/**
 * Function to initialize EventOS.
 *
 * This function should be called once before any other calls to the OS are made.
 * @param sysTicksPerOsTick The reload value to use witht he SysTick timer to generate EventOS tick.
 */
void os_Init(uint32_t sysTicksPerOsTick);

/**
 * Function to perform one execute action by the EventOS.
 *
 * This function is typically called within a 'while' loop in main().
 */
void os_Exec(void);

/** @} */

/** @defgroup DoAction Do actions (ASAP).
 *  Functions for executing actions in the near future.
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
 * Function to enqueue an action to be performed using an existing context
 * @param action The action to be performed (callback)
 * @param context Pointer to the context data that will be padded to the action when it is invoked
 */
void os_DoWith(os_action_t action, os_context_t context);

/** @} */

/** @defgroup DoAfter Do actions on a schedule (Timers).
 *  Functions for executing actions in the distant future (on a schedule)
 *  @{
 */

/**
 * Function to enqueue an action to be performed using a new context after the specified number of ticks have
 * elapsed.
 * @param action The action to be performed (callback)
 * @param contextSize The size of the context data required by the action
 * @param ticks The number of ticks to wait until the action is enqueued
 * @return Pointer to the context data that will be passed to the action when it is invoked
 */
os_context_t os_DoAfter(os_action_t action, uint32_t contextSize, uint32_t ticks);

/**
 * Function to enqueue an action to be performed using an existing context after the specified number of ticks have
 * @param action The action to be performed (callback)
 * @param context Pointer to the context data that will be padded to the action when it is invoked
 * @param ticks The number of ticks to wait until the action is enqueued
 */
void os_DoAfterWith(os_action_t action, os_context_t context, uint32_t ticks);

// TODO: A way to cancel a pending action e.g. cancel a future?

/** @} */

/**
 * @defgroup Publish-Subscribe Do actions using publish/subscribe (Topics).
 * Functions for executing actions using the public/subscribe design pattern.
 *
 * Definitions:
 * * Topic: a key value....(TODO)
 * * Subscriber:
 * * Publisher:
 * @{
 */

/**
 * Function to subscribe to a topic.
 * *Note:* Multiple subscriptions of an action to a topic is ignored, an action cannot subscribe to the same
 * topic more than once.
 * @param topic The topic to subscribe to.
 * @param action The action to invoke when the topic is published.
 */
void os_Subscribe(uint32_t topic, os_action_t action);

/**
 * Function to unsubscribe from a topic,
 * *Note:* Unsubscribing an action from a topic it is not subscribed to is ignored.
 * @param topic The topic to unsubscribe from.
 * @param action The action to unsubscribe from
 */
void os_Unsubscribe(uint32_t topic, os_action_t action);

/**
 * Function to unsubscribe all actions from a topic.
 * @param topic The topic to remove all subscriptions
 */
void os_UnsubscribeAll(uint32_t topic);

/**
 * Function to publish to a topic using a new context.
 * @param topic The topic to publish
 * @param contextSize The size of the context data required by the action
 * @return Pointer to the context data that will be passed to all subscribers.
 */
os_context_t os_Publish(uint32_t topic, uint32_t contextSize);

/**
 * Function to publish to a topic using an exsisting context.
 * @param topic The topic to publish
 * @param context Pointer to the context data that will be passed to all subscribers.
 */
void os_PublishWith(uint32_t topic, os_context_t context);

/** @} */

/** @defgroup Blackboard Long lived context management (Blackboard).
 *  Functions for managing long lived contexts.
 *  @{
 */

/**
 * **TODO** Function to post a context to a "blackboard" to be read by a action in the future.
 * @param key The key to access the item on the blackboard
 * @param context Pointer to the context to be stored on the blackboard
 * @param timeoutAction Action to perform when the context expires and is removed from the blackboard
 * @param ticks Number of ticks that the context can stay on the blackboard
 */
void os_PutContext(uint32_t key, os_context_t context, os_action_t timeoutAction, uint32_t ticks);

/**
 * **TODO** Function to remove a context in the blackboard.
 * TBD: How to handle keys that do have an entry on the blockboard. {NULL?, NotFoundAction?}
 * @param key The key to the item on the blackboard
 * @return Pointer to the context removed from the blackboard
 */
os_context_t os_GetContext(uint32_t key); // TODO

/** @} */

#endif
