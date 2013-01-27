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

/** MCU clock frequency in Hz, obtained from makefile. */
#define F_CPU   ADK_MCU_FREQ

#include <avr/io.h>

#ifndef __ASSEMBLER__

/** Wide character type. */
typedef __WCHAR_TYPE__ wchar_t;

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include <string.h>

#include <adk/types.h>

#endif /* __ASSEMBLER__ */

#ifndef __ASSEMBLER__
/** Get low-ordered byte of 16-bits integer. */
#define AVR_LO8(__value)                    ((__value) & 0xff)
/** Get high-ordered byte of 16-bits integer. */
#define AVR_HI8(__value)                    ((__value) >> 8)
#else /* __ASSEMBLER__ */
#define AVR_LO8(__value)                    lo8(__value)
#define AVR_HI8(__value)                    hi8(__value)
#endif /* __ASSEMBLER__ */

/** Set bit in destination 8-bits operand. */
#define AVR_BIT_SET8(__dst, __bit)          (__dst) = (__dst) | (u8)_BV(__bit)
/** Clear bit in destination 8-bits operand. */
#define AVR_BIT_CLR8(__dst, __bit)          (__dst) = (__dst) & (u8)~_BV(__bit)
/** Toggle bit in destination 8-bits operand. */
#define AVR_BIT_TOGGLE8(__dst, __bit)       (__dst) = (__dst) ^ (u8)~_BV(__bit)
/** Set bit to the specified value (zero/non-zero). */
#define AVR_BIT_COPY8(__dst, __bit, __value) { \
    if (__value) { \
        AVR_BIT_SET8(__dst, __bit); \
    } else { \
        AVR_BIT_CLR8(__dst, __bit); \
    } \
}
/** Get specified bit in source 8-bits operand. */
#define AVR_BIT_GET8(__src, __bit)          ((__src) & (u8)_BV(__bit))

#endif /* AVR_H_ */
