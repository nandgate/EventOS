/**
 * @file Critical.c
 * @brief No-op critical section implementation for the single-threaded POSIX demo.
 *
 * On real hardware, hal_CriticalBegin/End mask and unmask interrupts
 * (on the bluepill, all maskable IRQs via __disable_irq/__enable_irq)
 * so no ISR can preempt a timer-queue or FIFO mutation mid-update.
 * This POSIX port runs one thread: os_Tick() is called inline from
 * posix_Tick() between os_Exec() iterations, so there is no preemption
 * to guard against and these calls are no-ops.
 *
 * LIMITATION: any bug whose only symptom is an action-vs-ISR race (for
 * example, forgetting to protect a pointer splice in the timer queue)
 * will *not* reproduce in this demo. For a faithful concurrency model,
 * run the tick in a separate pthread (e.g. via timer_create with
 * SIGEV_THREAD) and implement these routines with a pthread_mutex.
 */

#include "hal/Critical.h"

void hal_CriticalInit(void) { }
void hal_CriticalBegin(void) { }
void hal_CriticalEnd(void) { }
