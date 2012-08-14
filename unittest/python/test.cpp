/* /ADK/unittest/python/test.cpp
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file test.cpp
 * Unit tests for embedded Python.
 */

#include <adk.h>
#include <adk_ut.h>

using namespace adk;

UT_TEST("Variables")
{
    py::Interpreter interpreter;
    {
        py::BlockingSection bs;
    }
    {
        py::ThreadLock lock;
    }

    py::Object res = py::Run("a = 237");
    UT(res.Get()) != UT_NULL;
}
UT_TEST_END
