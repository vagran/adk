/* /ADK/include/adk/utils.h
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file utils.h
 * TODO insert description here.
 */

#ifndef ADK_UTILS_H_
#define ADK_UTILS_H_

/** Number of bits in byte */
#define NBBY                        8

/** Get offset of member @a member in structure or class @a type. */
#ifndef OFFSETOF
#define OFFSETOF(type, member)      __builtin_offsetof(type, member)
#endif

#ifndef SIZEOF_ARRAY
#define SIZEOF_ARRAY(array)         (sizeof(array) / sizeof((array)[0]))
#endif

#ifndef __CONCAT2
#define __CONCAT2(x, y)             x##y
#endif
/** Macro for concatenating identifiers. */
#ifndef __CONCAT
#define __CONCAT(x, y)              __CONCAT2(x, y)
#endif

#ifndef __STR2
#define __STR2(x)                   # x
#endif
/** Macro for stringifying identifiers. */
#ifndef __STR
#define __STR(x)                    __STR2(x)
#endif

/** Generate file-scope unique identifier with a given prefix. */
#ifndef __UID
#define __UID(str)                  __CONCAT(str, __COUNTER__)
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
#ifndef UNUSED
#define UNUSED                      __attribute__((unused))
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
#define IS_POWER_OF_2(value)       ((((value) - 1) & (value)) == 0)
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

namespace adk {

/** Minimal value. */
template <typename T>
inline constexpr T
Min(T x, T y) { return MIN(x, y); }

/** Maximal value. */
template <typename T>
inline constexpr T
Max(T x, T y) { return MAX(x, y); }

/** Round up the value with specified alignment. */
template <typename T, typename Tal>
inline constexpr T
RoundUp(T size, Tal align) { return ROUND_UP(size, align); }

/** Round down the value with specified alignment. */
template <typename T, typename Tal>
inline constexpr T
RoundDown(T size, Tal align) { return ROUND_DOWN(size, align); }

/** Check if specified value is an integer power of two. */
template <typename T>
inline constexpr bool
IsPowerOf2(T value) { return IS_POWER_OF_2(value); }

/** Round up the value with specified alignment. Alignment must be an integer
 * power of two.
 */
template <typename T, typename Tal>
inline constexpr T
RoundUp2(T size, Tal align)
{
    return ROUND_UP2(size, align);
}

/** Round down the value with specified alignment. Alignment must be an integer
 * power of two.
 */
template <typename T, typename Tal>
inline constexpr T
RoundDown2(T size, Tal align)
{
    return ROUND_DOWN2(size, align);
}

/** Bit-rotate value left by specified number of bits.
 *
 * @param value Value to rotate.
 * @param numBits Number of bits to rotate.
 * @return Rotation result.
 */
template <typename T>
inline constexpr T
RotL(const T value, const size_t numBits)
{
    return (value << numBits) | (value >> (sizeof(value) * NBBY - numBits));
}

/** Bit-rotate value right by specified number of bits.
 *
 * @param value Value to rotate.
 * @param numBits Number of bits to rotate.
 * @return Rotation result.
 */
template <typename T>
inline constexpr T
RotR(const T value, const size_t numBits)
{
    return (value >> numBits) | (value << (sizeof(value) * NBBY - numBits));
}

/** Helper structure for accessing unaligned data fields.
 * @see GetUnaligned
 * @see PutUnaligned
 */
template <typename T>
struct UnalignedData {
    T value;
} __PACKED;

/** Get unaligned data at specified location. Data type indicated by template
 * argument @a T.
 * @param p Location of unaligned data to retrieve.
 * @return Retrieved data from specified location.
 */
template <typename T>
inline constexpr T
GetUnaligned(const void *p)
{
    return static_cast<const UnalignedData<T> *>(p)->value;
}

/** Put unaligned data at specified location. Data type indicated by template
 * argument @a T.
 * @param p Location of unaligned data to put.
 * @param value Value to put.
 * @return Put data to specified location.
 */
template <typename T>
inline void
PutUnaligned(T value, void *p)
{
    static_cast<const UnalignedData<T> *>(p)->value = value;
}

} /* namespace adk */

#endif /* ADK_UTILS_H_ */
