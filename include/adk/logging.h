/* /ADK/include/adk/logging.h
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file logging.h
 * Generic logging framework.
 */

#ifndef ADK_LOGGING_H_
#define ADK_LOGGING_H_

#ifdef UNITTEST

#define ADK_CRITICAL(msg, ...) UT_TRACE("[CRIT] " msg, ## __VA_ARGS__)
#define ADK_ERROR(msg, ...) UT_TRACE("[ERROR] " msg, ## __VA_ARGS__)
#define ADK_WARNING(msg, ...) UT_TRACE("[WARN] " msg, ## __VA_ARGS__)
#define ADK_INFO(msg, ...) UT_TRACE("[INFO] " msg, ## __VA_ARGS__)

#else /* UNITTEST */

#ifdef ADK_PLATFORM_AVR

//XXX

#else /* ADK_PLATFORM_AVR */

#define ADK_CRITICAL(msg, ...) g_critical("%s:%d: " msg, __FILE__, __LINE__, ## __VA_ARGS__)
#define ADK_ERROR(msg, ...) g_error("%s:%d: " msg, __FILE__, __LINE__, ## __VA_ARGS__)
#define ADK_WARNING(msg, ...) g_warning("%s:%d: " msg, __FILE__, __LINE__, ## __VA_ARGS__)
#define ADK_INFO(msg, ...) g_message("%s:%d: " msg, __FILE__, __LINE__, ## __VA_ARGS__)

/** Get description for last system error. */
std::string
GetSystemError();

#endif /* ADK_PLATFORM_AVR */

#endif /* UNITTEST */

#endif /* ADK_LOGGING_H_ */
