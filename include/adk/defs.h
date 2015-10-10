/* This file is a part of ADK library.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See LICENSE file for copyright details.
 */

/** @file defs.h
 * Common ADK macro definitions.
 */

#ifndef ADK_DEFS_H_
#define ADK_DEFS_H_

/* ADK platform numeric identifiers to use in preprocessor directives. */
#define ADK_PLATFORM_ID_AVR         0
#define ADK_PLATFORM_ID_LINUX32     1
#define ADK_PLATFORM_ID_LINUX64     2
#define ADK_PLATFORM_ID_WIN32       3
#define ADK_PLATFORM_ID_WIN64       4

/** Check if platform is AVR. */
#define ADK_PLATFORM_IS_AVR(id)     ((id) == ADK_PLATFORM_ID_AVR)
/** Check if platform is Linux. */
#define ADK_PLATFORM_IS_LINUX(id) \
    ((id) == ADK_PLATFORM_ID_LINUX32 || (id) == ADK_PLATFORM_ID_LINUX64)
/** Check if platform is Windows. */
#define ADK_PLATFORM_IS_WINDOWS(id) \
    ((id) == ADK_PLATFORM_ID_WIN32 || (id) == ADK_PLATFORM_ID_WIN64)

#define __NOP(x)                    x

#ifndef __CONCAT2
#define __CONCAT2(x, y)             x##y
#endif
/** Macro for concatenating identifiers. */
#ifdef __CONCAT
#undef __CONCAT
#endif
#define __CONCAT(x, y)              __CONCAT2(x, y)

#ifndef __STR2
#define __STR2(x)                   # x
#endif
/** Macro for stringifying identifiers. */
#ifndef __STR
#define __STR(x)                    __STR2(x)
#endif

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

/** Number of bits in byte */
#define NBBY                        8

/** Get offset of member @a member in structure or class @a type. */
#ifndef OFFSETOF
#define OFFSETOF(type, member)      __builtin_offsetof(type, member)
#endif

#ifndef SIZEOF_ARRAY
#define SIZEOF_ARRAY(array)         (sizeof(array) / sizeof((array)[0]))
#endif

#define __UID2(str, counter)        __CONCAT(str, counter)

/** Generate file-scope unique identifier with a given prefix. */
#ifndef __UID
#define __UID(str)                  __UID2(str, __COUNTER__)
#endif

/** Give a hint for the compiler that a given conditional statement is likely to
 * be true.
 *
 * Usage example:
 * @code
 * if (LIKELY(someCondition)) { ... }
 * @endcode
 */
#ifndef LIKELY
#define LIKELY(condition)           __builtin_expect(!!(condition), 1)
#endif
/** Give a hint for the compiler that a given conditional statement is likely to
 * be false.
 *
 * Usage example:
 * @code
 * if (UNLIKELY(someCondition)) { ... }
 * @endcode
 */
#ifndef UNLIKELY
#define UNLIKELY(condition)         __builtin_expect(!!(condition), 0)
#endif

/** Macro for marking unused parameters.
 *
 * Usage example:
 * @code
 * int SomeFunction(int UNUSED someParam) { ... }
 * @endcode
 */
#ifndef __UNUSED
#define __UNUSED                    __attribute__((unused))
#endif

/* Shortcuts for various compiler attributes */
#define __PACKED                    __attribute__((packed))
#define __NORETURN                  __attribute__ ((noreturn))
#define __NOINLINE                  __attribute__ ((noinline))

/** Provide binary constants in the code. */
#ifndef BIN
#define BIN(x) ((x & 0x1) | ((x & 0x10) ? 0x2 : 0) | \
    ((x & 0x100) ? 0x4 : 0) | ((x & 0x1000) ? 0x8 : 0) | \
    ((x & 0x10000) ? 0x10 : 0) | ((x & 0x100000) ? 0x20 : 0) | \
    ((x & 0x1000000) ? 0x40 : 0) | ((x & 0x10000000) ? 0x80 : 0))
#endif

/** Minimal value. */
#ifndef MIN
#define MIN(x, y)                   ((x) < (y) ? (x) : (y))
#endif
/** Maximal value. */
#ifndef MAX
#define MAX(x, y)                   ((x) > (y) ? (x) : (y))
#endif

/** Sign function. */
#ifndef SIGN
#define SIGN(x)                     ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))
#endif

/** Round up the value with specified alignment. */
#ifndef ROUND_UP
#define ROUND_UP(size, align)      (((size) + (align) - 1) / (align) * (align))
#endif
/** Round down the value with specified alignment. */
#ifndef ROUND_DOWN
#define ROUND_DOWN(size, align)    ((size) / (align) * (align))
#endif
/** Check if value is power of 2. */
#ifndef IS_POWER_OF_2
#define IS_POWER_OF_2(value)       ((value) && (((value) - 1) & (value)) == 0)
#endif

/** Round up the value with specified alignment. Alignment must be an integer
 * power of two.
 */
#ifndef ROUND_UP2
#define ROUND_UP2(size, align)     (((size) + (align) - 1) & (~((align) - 1)))
#endif
/** Round down the value with specified alignment. Alignment must be an integer
 * power of two.
 */
#ifndef ROUND_DOWN2
#define ROUND_DOWN2(size, align)   ((size) & (~((align) - 1)))
#endif

#endif /* ADK_DEFS_H_ */
