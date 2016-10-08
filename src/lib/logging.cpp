/* This file is a part of ADK library.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See LICENSE file for copyright details.
 */

/** @file logging.cpp */

#include <adk.h>

using namespace adk;

namespace {

void
GLogFuncImpl(Log::Level level, const char *msg, va_list args)
{
    GLogLevelFlags flags;
    switch (level) {
    case Log::Level::CRITICAL:
        flags = G_LOG_LEVEL_CRITICAL;
        break;
    case Log::Level::ERROR:
        flags = G_LOG_LEVEL_ERROR;
        break;
    case Log::Level::WARNING:
        flags = G_LOG_LEVEL_WARNING;
        break;
    case Log::Level::INFO:
        flags = G_LOG_LEVEL_INFO;
        break;
    case Log::Level::DEBUG:
        flags = G_LOG_LEVEL_DEBUG;
        break;
    default:
        ADK_EXCEPTION(InternalErrorException,
                      "Unrecognized log level: " << static_cast<int>(level));
    }
    g_logv("ADK", flags, msg, args);
}

} /* anonymuous namespace */

Log::LogFunc adk::GLogFunc = GLogFuncImpl;

Log Log::instance;

Log::Log():
    logFunc(GLogFunc)
{}

void
Log::Write(Level level, const char *fmt, ...)
{
    if (level > instance.level) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    instance.WriteV(level, fmt, args);
    va_end(args);
}

void
Log::WriteV(Level level, const char *fmt, va_list args)
{
    instance.logFunc(level, fmt, args);
}

Log::LogFunc
Log::GetLogFunc()
{
    return instance.logFunc;
}

void
Log::SetLogFunc(LogFunc logFunc)
{
    instance.logFunc = logFunc;
}

void
Log::SetLevel(Level level)
{
    instance.level = level;
}

std::string
Log::GetSystemError()
{
    int code = errno;
    char buf[1024];
    const char *desc = strerror_r(code, buf, sizeof(buf));

    std::stringstream ss;
    ss << code << " - " << desc;
    return ss.str();
}

int
Log::GetSystemErrorCode()
{
    return errno;
}

using Ms = std::chrono::milliseconds;
using Clock = std::chrono::steady_clock;
template<class Duration>
using TimePoint = std::chrono::time_point<Clock, Duration>;

std::string
Log::GetSystemTime()
{
    Clock::time_point now = Clock::now();
    TimePoint<Ms> tp = std::chrono::time_point_cast<Ms>(now);
    char buf[64];
    snprintf(buf, sizeof(buf), "%.3f", (double)tp.time_since_epoch().count() / 1000.0);
    return buf;
}

const char *
Log::GetFileBasename(const char *path)
{
    const char *lastCandidate = nullptr;
    for (const char *p = path; *p; p++) {
        if (*p == '/') {
            lastCandidate = p + 1;
        }
    }
    return lastCandidate ? lastCandidate : path;
}
