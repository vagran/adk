/* /ADK/unittest/python/test.cpp
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file test.cpp
 * Unit tests for embedded Python.
 */

#include <adk.h>
#include <adk_ut.h>

using namespace adk;

UT_TEST("Basic functionality")
{
    Xml xml;
    xml.Load(GetResource("test.xml").GetString());
}
