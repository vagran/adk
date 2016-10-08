/* This file is a part of ADK library.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See LICENSE file for copyright details.
 */

/** @file logging.h
 * Generic logging framework.
 */

#ifndef ADK_LOGGING_H_
#define ADK_LOGGING_H_

//XXX needs redesign

#ifndef ADK_PLATFORM_AVR

namespace adk {

class Log {
public:
    enum class Level {
        CRITICAL,
        ERROR,
        WARNING,
        INFO,
        DEBUG
    };

    using LogFunc = std::function<void(Level, const char *, va_list)>;

    static void
    Write(Level level, const char *fmt, ...) __FORMAT(printf, 2, 3);

    static void
    WriteV(Level level, const char *fmt, va_list args) __FORMAT(printf, 2, 0);

    static LogFunc
    GetLogFunc();

    static void
    SetLogFunc(LogFunc logFunc);

    static void
    SetLevel(Level level);

    /** Get error code for the last system error. */
    static int
    GetSystemErrorCode();

    /** Get description for last system error. */
    static std::string
    GetSystemError();

    static std::string
    GetSystemTime();

    static const char *
    GetFileBasename(const char *path);

private:
    static Log instance;

    LogFunc logFunc;
    Level level = Level::DEBUG;

    Log();
};

/** Log function backed by g_log(). */
extern Log::LogFunc GLogFunc;

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

#define ADK_CRITICAL(__msg, ...) adk::Log::Write( \
    adk::Log::Level::CRITICAL, "%s:%d: " __msg, \
    adk::Log::GetFileBasename(__FILE__), __LINE__, ## __VA_ARGS__)

#define ADK_ERROR(__msg, ...) adk::Log::Write( \
    adk::Log::Level::ERROR, "%s:%d: " __msg, \
    adk::Log::GetFileBasename(__FILE__), __LINE__, ## __VA_ARGS__)

#define ADK_WARNING(__msg, ...) adk::Log::Write( \
    adk::Log::Level::WARNING, "%s:%d: " __msg, \
    adk::Log::GetFileBasename(__FILE__), __LINE__, ## __VA_ARGS__)

#define ADK_INFO(__msg, ...) adk::Log::Write( \
    adk::Log::Level::INFO, "%s:%d: " __msg, \
    adk::Log::GetFileBasename(__FILE__), __LINE__, ## __VA_ARGS__)

#define ADK_DEBUG(__msg, ...) adk::Log::Write( \
    adk::Log::Level::DEBUG, "%s:%d: " __msg, \
    adk::Log::GetFileBasename(__FILE__), __LINE__, ## __VA_ARGS__)

#define ADK_LOG(__level, __msg, ...) adk::Log::Write( \
    (__level), "%s:%d: " __msg, \
    adk::Log::GetFileBasename(__FILE__), __LINE__, ## __VA_ARGS__)

#endif /* ADK_PLATFORM_AVR */

#endif /* UNITTEST */

#endif /* ADK_LOGGING_H_ */
