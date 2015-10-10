/* This file is a part of ADK library.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See LICENSE file for copyright details.
 */

/** @file logging.cpp */

#include <adk.h>

using namespace adk;

std::string
adk::GetSystemError()
{
    int code = errno;
    char buf[1024];
    const char *desc = strerror_r(code, buf, sizeof(buf));

    std::stringstream ss;
    ss << code << " - " << desc;
    return ss.str();
}

int
adk::GetSystemErrorCode()
{
    return errno;
}

using Ms = std::chrono::milliseconds;
using Clock = std::chrono::steady_clock;
template<class Duration>
using TimePoint = std::chrono::time_point<Clock, Duration>;

std::string
adk::GetSystemTime()
{
    Clock::time_point now = Clock::now();
    TimePoint<Ms> tp = std::chrono::time_point_cast<Ms>(now);
    char buf[64];
    snprintf(buf, sizeof(buf), "%.3f", (double)tp.time_since_epoch().count() / 1000.0);
    return buf;
}
