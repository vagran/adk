/* /ADK/include/adk/utils.h
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file utils.h
 * TODO insert description here.
 */

#ifndef ADK_UTILS_H_
#define ADK_UTILS_H_

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
