/* This file is a part of ADK library.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file endian.h
 * Byte-order related conversions.
 */

#ifndef ENDIAN_H_
#define ENDIAN_H_

/* Not in adk namespace for consistency with types.h */

#ifdef __cplusplus

namespace internal {

/** Value for testing system endianness. */
static const u16 endianness = 0x0102;

}

/** Check if the system is little-endian. */
constexpr bool
IsSystemLe()
{
    return *reinterpret_cast<const uint8_t *>(&internal::endianness) == 0x02;
}

/** Check if the system is big-endian. */
constexpr bool
IsSystemBe()
{
    return !IsSystemLe();
}

/** Swap bytes in 16-bits integer value. */
#define ADK_BSWAP16(x)      (((x) >> 8) | ((x) << 8))
/** Swap bytes in 32-bits integer value. */
#define ADK_BSWAP32(x)      __builtin_bswap32(x)
/** Swap bytes in 64-bits integer value. */
#define ADK_BSWAP64(x)      __builtin_bswap64(x)

/** Stub for easier conversion functions generalization. */
template <typename T>
constexpr T
ConvertBe8(T x)
{
    return x;
}

/** Convert 16 bits value byte order from BE to host byte order and vice versa.
 * Template parameters:
 * - *T* 16-bits integer type.
 * @param x Value to convert.
 */
template <typename T>
constexpr T
ConvertBe16(T x)
{
    return IsSystemBe() ? x : ADK_BSWAP16(x);
}

/** Convert 32 bits value byte order from BE to host byte order and vice versa.
 * Template parameters:
 * - *T* 32-bits integer type.
 * @param x Value to convert.
 */
template <typename T>
constexpr T
ConvertBe32(T x)
{
    return IsSystemBe() ? x : ADK_BSWAP32(x);
}

/** Convert 64 bits value byte order from BE to host byte order and vice versa.
 * Template parameters:
 * - *T* 64-bits integer type.
 * @param x Value to convert.
 */
template <typename T>
constexpr T
ConvertBe64(T x)
{
    return IsSystemBe() ? x : ADK_BSWAP64(x);
}

/** Stub for easier conversion functions generalization. */
template <typename T>
constexpr T
ConvertLe8(T x)
{
    return x;
}

/** Convert 16 bits value byte order from LE to host byte order and vice versa.
 * Template parameters:
 * - *T* 16-bits integer type.
 * @param x Value to convert.
 */
template <typename T>
constexpr T
ConvertLe16(T x)
{
    return IsSystemLe() ? x : ADK_BSWAP16(x);
}

/** Convert 32 bits value byte order from LE to host byte order and vice versa.
 * Template parameters:
 * - *T* 32-bits integer type.
 * @param x Value to convert.
 */
template <typename T>
constexpr T
ConvertLe32(T x)
{
    return IsSystemLe() ? x : ADK_BSWAP32(x);
}

/** Convert 64 bits value byte order from LE to host byte order and vice versa.
 * Template parameters:
 * - *T* 64-bits integer type.
 * @param x Value to convert.
 */
template <typename T>
constexpr T
ConvertLe64(T x)
{
    return IsSystemLe() ? x : ADK_BSWAP64(x);
}

/** Stub for easier conversion functions generalization. */
template <typename T>
constexpr T
ConvertNh8(T x)
{
    return x;
}

/** Convert 16 bits value byte order from network to host byte order and vice
 * versa.
 * Template parameters:
 * - *T* 16-bits integer type.
 * @param x Value to convert.
 */
template <typename T>
constexpr T
ConvertNh16(T x)
{
    return ConvertBe16(x);
}

/** Convert 32 bits value byte order from network to host byte order and vice
 * versa.
 * Template parameters:
 * - *T* 32-bits integer type.
 * @param x Value to convert.
 */
template <typename T>
constexpr T
ConvertNh32(T x)
{
    return ConvertBe32(x);
}

/** Convert 64 bits value byte order from network to host byte order and vice
 * versa.
 * Template parameters:
 * - *T* 64-bits integer type.
 * @param x Value to convert.
 */
template <typename T>
constexpr T
ConvertNh64(T x)
{
    return ConvertBe64(x);
}

/// @{
/** Definitions for byte order conversions for all integer types. */
#define __ADK_BO_INT(type_size)     i ## type_size
#define __ADK_BO_UINT(type_size)    u ## type_size

#define _ADK_DEF_BO_CONV(type_size, type_name) \
    /** Convert value from network to host byte order. */ \
    constexpr type_name \
    Ntoh(type_name x) \
    { \
        return ConvertNh ## type_size (x); \
    } \
    \
    /** Convert value from host to network byte order. */ \
    constexpr type_name \
    Hton(type_name x) \
    { \
        return ConvertNh ## type_size (x); \
    } \
    \
    /** Convert value from LE to host byte order and vice versa. */ \
    constexpr type_name \
    Le(type_name x) \
    { \
        return ConvertLe ## type_size (x); \
    } \
    \
    /** Convert value from BE to host byte order and vice versa. */ \
    constexpr type_name \
    Be(type_name x) \
    { \
        return ConvertBe ## type_size (x); \
    } \

#define ADK_DEF_BO_CONV(type_size) \
    _ADK_DEF_BO_CONV(type_size, __ADK_BO_INT(type_size)) \
    _ADK_DEF_BO_CONV(type_size, __ADK_BO_UINT(type_size))

/// @}

ADK_DEF_BO_CONV(8)
ADK_DEF_BO_CONV(16)
ADK_DEF_BO_CONV(32)
ADK_DEF_BO_CONV(64)

/** Convert float type from host to network format. */
constexpr float
Hton(float x)
{
    //XXX
    return x;
}

/** Convert float type from network to host format. */
constexpr float
Ntoh(float x)
{
    //XXX
    return x;
}

/** Convert double type from host to network format. */
constexpr double
Hton(double x)
{
    //XXX
    return x;
}

/** Convert double type from network to host format. */
constexpr double
Ntoh(double x)
{
    //XXX
    return x;
}

/** Convert float type from LE to host format. */
constexpr float
Le(float x)
{
    //XXX
    return x;
}

/** Convert float type from BE to host format. */
constexpr float
Be(float x)
{
    //XXX
    return x;
}

/** Convert float type from LE to host format. */
constexpr double
Le(double x)
{
    //XXX
    return x;
}

/** Convert float type from BE to host format. */
constexpr double
Be(double x)
{
    //XXX
    return x;
}

namespace internal {

/** Helper class for LE-order conversions. */
class LeConverter {
public:
    template <typename T>
    static constexpr T
    Convert(T value)
    {
        return Le(value);
    }
};

/** Helper class for BE-order conversions. */
class BeConverter {
public:
    template <typename T>
    static constexpr T
    Convert(T value)
    {
        return Be(value);
    }
};

} /* namespace internal */

/** Helper class for byte-order-dependent value representation. */
template <typename T, class Converter>
class BoValue {
public:
    /** Construct value.
     *
     * @param value Value in host byte order.
     */
    BoValue(T value):
        value(Converter::Convert(value))
    {}

    /** Assign new value.
     *
     * @param value Value in host byte order.
     */
    BoValue &
    operator =(T value)
    {
        this->value = Converter::Convert(value);
        return *this;
    }

    /** Cast to underlying type.
     *
     * @return Value in host byte order.
     */
    operator T() const
    {
        return Converter::Convert(value);
    }

    /** Get the value of underlying type.
     *
     * @return Value in host byte order.
     */
    T
    Get() const
    {
        return Converter::Convert(value);
    }

private:
    /** Stored value (in wire byte order). */
    T value;
} __PACKED;

/** Little-endian value wrapper.
 * @param T Underlying primitive type.
 */
template <typename T>
using LeValue = BoValue<T, internal::LeConverter>;

/** Big-endian value wrapper.
 * @param T Underlying primitive type.
 */
template <typename T>
using BeValue = BoValue<T, internal::BeConverter>;

/// @{
/** Standard primitive types for little-endian byte order. */
typedef LeValue<i8> LeI8;
typedef LeValue<u8> LeU8;
typedef LeValue<i16> LeI16;
typedef LeValue<u16> LeU16;
typedef LeValue<i32> LeI32;
typedef LeValue<u32> LeU32;
typedef LeValue<i64> LeI64;
typedef LeValue<u64> LeU64;
typedef LeValue<float> LeFloat;
typedef LeValue<double> LeDouble;
/// @}


/// @{
/** Standard primitive types for big-endian byte order. */
typedef BeValue<i8> BeI8;
typedef BeValue<u8> BeU8;
typedef BeValue<i16> BeI16;
typedef BeValue<u16> BeU16;
typedef BeValue<i32> BeI32;
typedef BeValue<u32> BeU32;
typedef BeValue<i64> BeI64;
typedef BeValue<u64> BeU64;
typedef BeValue<float> BeFloat;
typedef BeValue<double> BeDouble;
/// @}

#else /* __cplusplus */

typedef i8 LeI8;
typedef u8 LeU8;
typedef i16 LeI16;
typedef u16 LeU16;
typedef i32 LeI32;
typedef u32 LeU32;
typedef i64 LeI64;
typedef u64 LeU64;
typedef float LeFloat;
typedef double LeDouble;

typedef i8 BeI8;
typedef u8 BeU8;
typedef i16 BeI16;
typedef u16 BeU16;
typedef i32 BeI32;
typedef u32 BeU32;
typedef i64 BeI64;
typedef u64 BeU64;
typedef float BeFloat;
typedef double BeDouble;

#endif /* __cplusplus */

#endif /* ENDIAN_H_ */
