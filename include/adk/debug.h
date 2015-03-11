/* This file is a part of ADK library.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file debug.h
 * Debug features for ADK library.
 */

#ifndef ADK_DEBUG_H_
#define ADK_DEBUG_H_

#ifdef UNITTEST

#define ASSERT_IMPL(condStr) do { \
    UT_FAIL("Assert failed: " condStr); \
} while (false)

#elif defined(ADK_PLATFORM_AVR)

#define ASSERT_IMPL(condStr) do { \
    /* XXX throw assertion failure on AVR platform. */
} while (false)

#else

#define ASSERT_IMPL(condStr) do { \
    ADK_EXCEPTION(adk::InternalErrorException, "Assert failed: " condStr); \
} while (false)

#endif /* UNITTEST */

#ifdef DEBUG

/** Verify that expression is true in debug build. In release build the
 * expression is not evaluated.
 */
#define ASSERT(x) do { \
    if (UNLIKELY(!(x))) { \
        ADK_CRITICAL("Assert failed: '%s'", # x); \
        ASSERT_IMPL(# x); \
    } \
} while (false)

/** Verify that expression is equal to the expected value in debug build. In
 * release build the expression is evaluated but not verified.
 */
#define VERIFY(x, expected) do { \
    if (UNLIKELY((x) != (expected))) { \
        ADK_CRITICAL("Verification failed: '%s'", # x " == " # expected); \
        ASSERT_IMPL(# x " == " # expected); \
    } \
} while (false)

#else /* DEBUG */

#define ASSERT(x)

#define VERIFY(x, expected)       x

#endif /* DEBUG */

#define ENSURE(x) do { \
    if (UNLIKELY(!(x))) { \
        ADK_CRITICAL("Ensure failed: '%s'", # x); \
        ADK_EXCEPTION(adk::InternalErrorException, "Ensure failed: " # x); \
    } \
} while(false)

#endif /* ADK_DEBUG_H_ */
