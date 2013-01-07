/* /ADK/include/adk/avr.h
 *
 * This file is a part of 'ADK' project.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file avr.h
 * AVR-target specific definitions.
 */

#ifndef AVR_H_
#define AVR_H_

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#include <adk/types.h>

/** Set bit in destination 8-bits operand. */
#define AVR_BIT_SET8(__dst, __bit) (__dst) = (u8)(__dst) | (u8)(1 << (__bit))
/** Clear bit in destination 8-bits operand. */
#define AVR_BIT_CLR8(__dst, __bit) (__dst) = (u8)(__dst) & (u8)~(1 << (__bit))
/** Toggle bit in destination 8-bits operand. */
#define AVR_BIT_TOGGLE8(__dst, __bit) (__dst) = (u8)(__dst) ^ (u8)~(1 << (__bit))

#ifdef ADK_AVR_USE_USB
#include <adk/avr/usb.h>
#endif /* ADK_AVR_USE_USB */

#endif /* AVR_H_ */
