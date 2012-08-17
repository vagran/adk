/* /ADK/include/adk/defs.h
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
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

//XXX should be removed after Eclipse will have c++11 supported
#ifdef __CDT_PARSER__
#define constexpr
#define nullptr 0
#define noexcept
#endif /* __CDT_PARSER__ */

#endif /* ADK_DEFS_H_ */