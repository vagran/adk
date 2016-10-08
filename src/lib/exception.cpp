/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2016, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See LICENSE file for copyright details.
 */

/** @file exception.cpp */


#include <adk.h>

using namespace adk;

void
Exception::_StrFileLine()
{
    std::stringstream ss;
    ss << "[" << Log::GetFileBasename(_file) << ":" << _line << "]: ";
    ss << _msg;
    _msg = ss.str();
}
