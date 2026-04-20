/**
 * @file hal/Critical.h
 * @brief This module provides critical section behavior on a per target system basis.
 */

#pragma once

/**
 * @brief Initialize the critical section support system. This should be called once at startup.
 * @ingroup SUPPORT_FN
 */
void hal_CriticalInit();

/**
 * @brief  Begin a critical section on a target. Once a critical section is entered execution continues until the end
 * of the critical section with out interrupts, etc. Critical sections can be nested.
 * @ingroup SUPPORT_FN
 */
void hal_CriticalBegin(void);

/**
 * @brief  End a critical section on a target. Critical sections can be nested.
 * @ingroup SUPPORT_FN
 */
void hal_CriticalEnd(void);