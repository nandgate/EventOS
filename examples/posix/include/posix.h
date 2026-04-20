/**
 * @file posix.h
 * @brief POSIX host platform support for EventOS (single-threaded demo).
 *
 * This port runs EventOS on the host so examples can be exercised
 * without bringing up hardware. LEDs print to stdout with a wall-clock
 * timestamp, and SysTick is replaced by `posix_Tick()`, which the
 * application calls from its exec loop to advance the OS tick counter.
 *
 * The port is intentionally single-threaded. See `Critical.c` for the
 * limitations this implies and for what a more faithful port would do.
 */

#pragma once

/**
 * @brief Bring the host platform up to where EventOS is ready to run.
 *
 * Snapshots the start time, initializes the LED printer, critical-section
 * HAL (no-op on POSIX), and the OS core. After this returns, post initial
 * actions and enter the `posix_Tick() / os_Exec()` loop (see Blinky-posix).
 */
void posix_Init(void);

/**
 * @brief Advance os_Tick() to match wall-clock time.
 *
 * Intended to be called once per iteration of the exec loop. Counts
 * how many whole milliseconds have elapsed since the last call and
 * invokes `os_Tick()` that many times. Because it runs inline between
 * actions, tick delivery is cooperative — if an action takes longer
 * than 1 ms, subsequent ticks are coalesced (the FIFO catches up, but
 * inter-tick ordering is lost).
 */
void posix_Tick(void);
