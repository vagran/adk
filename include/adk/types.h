/* This file is a part of ADK library.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file types.h
 * ADK common data types.
 */

#ifndef ADK_TYPES_H_
#define ADK_TYPES_H_

typedef int8_t              i8; /**< Signed 8 bits integer */
typedef int16_t             i16; /**< Signed 16 bits integer */
typedef int32_t             i32; /**< Signed 32 bits integer */
typedef int64_t             i64; /**< Signed 64 bits integer */
typedef uint8_t             u8; /**< Unsigned 8 bits integer */
typedef uint16_t            u16; /**< Unsigned 16 bits integer */
typedef uint32_t            u32; /**< Unsigned 32 bits integer */
typedef uint64_t            u64; /**< Unsigned 64 bits integer */

#ifdef ADK_PLATFORM_AVR
typedef u8                  bool_t; /** Boolean value. */
#endif /* ADK_PLATFORM_AVR */

#define MAX_U8              UCHAR_MAX
#define MAX_U16             USHRT_MAX
#define MAX_U32             UINT_MAX
#define MAX_U64             ULLONG_MAX

#define MAX_I8              CHAR_MAX
#define MAX_I16             SHRT_MAX
#define MAX_I32             INT_MAX
#define MAX_I64             LLONG_MAX

#endif /* ADK_TYPES_H_ */
