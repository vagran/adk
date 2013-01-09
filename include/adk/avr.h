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
#define AVR_BIT_SET8(__dst, __bit) (__dst) = (__dst) | (u8)(1 << (__bit))
/** Clear bit in destination 8-bits operand. */
#define AVR_BIT_CLR8(__dst, __bit) (__dst) = (__dst) & (u8)~(1 << (__bit))
/** Toggle bit in destination 8-bits operand. */
#define AVR_BIT_TOGGLE8(__dst, __bit) (__dst) = (__dst) ^ (u8)~(1 << (__bit))
/** Set bit to the specified value (zero/non-zero). */
#define AVR_BIT_COPY8(__dst, __bit, __value) { \
    if (__value) { \
        AVR_BIT_SET8(__dst, __bit); \
    } else { \
        AVR_BIT_CLR8(__dst, __bit); \
    } \
}
/** Get specified bit in source 8-bits operand. */
#define AVR_BIT_GET8(__src, __bit) ((__src) & (u8)(1 << (__bit)))

#ifdef ADK_AVR_USE_USB
#include <adk/avr/usb.h>
#endif /* ADK_AVR_USE_USB */

#endif /* AVR_H_ */
