/* /ADK/src/lib/logging.cpp
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
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
