/* /ADK/include/adk/debug.h
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
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
    ADK_EXCEPTION(adk::Exception, "Assert failed: " condStr); \
} while (false)

#endif /* UNITTEST */

#ifdef DEBUG

#define ASSERT(x) do { \
    if (UNLIKELY(!(x))) { \
        ADK_CRITICAL("Assert failed: '%s'", # x); \
        ASSERT_IMPL(# x); \
    } \
} while (false)

#else /* DEBUG */

#define ASSERT(x)

#endif /* DEBUG */


#endif /* ADK_DEBUG_H_ */
