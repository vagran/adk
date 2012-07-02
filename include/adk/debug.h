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

#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef UNITTEST

#define ASSERT_IMPL() do { \
    UT_FAIL("Assert failed"); \
} while (false)

#else /* UNITTEST */

#define ASSERT_IMPL() do { \
    throw "Assert failed"; \
} while (false)

#endif /* UNITTEST */

#ifdef DEBUG

#define ASSERT(x) do { \
    if (UNLIKELY(!(x))) { \
        ADK_CRITICAL("Assert failed: '%s'", # x); \
        ASSERT_IMPL(); \
    } \
} while (false)

#else /* DEBUG */

#define ASSERT(x)

#endif /* DEBUG */


#endif /* DEBUG_H_ */
