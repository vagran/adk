/* This file is a part of ADK library.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file logging.h
 * Generic logging framework.
 */

#ifndef ADK_LOGGING_H_
#define ADK_LOGGING_H_

#ifndef ADK_PLATFORM_AVR

namespace adk {

/** Get error code for the last system error. */
int
GetSystemErrorCode();

/** Get description for last system error. */
std::string
GetSystemError();

std::string
GetSystemTime();

} /* namespace adk */

#endif /* !ADK_PLATFORM_AVR */

#ifdef UNITTEST

#define ADK_CRITICAL(msg, ...) UT_TRACE("[CRIT] " msg, ## __VA_ARGS__)
#define ADK_ERROR(msg, ...) UT_TRACE("[ERROR] " msg, ## __VA_ARGS__)
#define ADK_WARNING(msg, ...) UT_TRACE("[WARN] " msg, ## __VA_ARGS__)
#define ADK_INFO(msg, ...) UT_TRACE("[INFO] " msg, ## __VA_ARGS__)

#else /* UNITTEST */

#ifdef ADK_PLATFORM_AVR

//XXX

#else /* ADK_PLATFORM_AVR */

#define ADK_CRITICAL(msg, ...) g_critical("[%s] %s:%d: " msg, \
        adk::GetSystemTime().c_str(), __FILE__, __LINE__, ## __VA_ARGS__)
#define ADK_ERROR(msg, ...) g_error("[%s] %s:%d: " msg, \
        adk::GetSystemTime().c_str(), __FILE__, __LINE__, ## __VA_ARGS__)
#define ADK_WARNING(msg, ...) g_warning("[%s] %s:%d: " msg, \
        adk::GetSystemTime().c_str(), __FILE__, __LINE__, ## __VA_ARGS__)
#define ADK_INFO(msg, ...) g_message("[%s] %s:%d: " msg, \
        adk::GetSystemTime().c_str(), __FILE__, __LINE__, ## __VA_ARGS__)

#endif /* ADK_PLATFORM_AVR */

#endif /* UNITTEST */

#endif /* ADK_LOGGING_H_ */
