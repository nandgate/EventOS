/**
 * @file bluepill.h
 * @brief BluePill (STM32F103) platform support for EventOS.
 */

#pragma once

#include <stdint.h>

/**
 * @brief Bring the BluePill up to a point where EventOS is ready to run.
 *
 * Configures the core clock cache, LEDs, critical-section HAL, the OS,
 * and finally the SysTick timer (in that order). After this returns,
 * the application may post its first actions and enter `os_Exec()` loop.
 *
 * Examples that want to expose the init sequence explicitly (see Blinky)
 * may bypass this and call the individual steps themselves.
 */
void bluepill_Init(void);

/**
 * @brief Initialize the SysTick timer to generate EventOS ticks.
 * @param sysTicksPerOsTick The SysTick reload value (system clocks per OS tick).
 */
void bluepill_SysTickInit(uint32_t sysTicksPerOsTick);
