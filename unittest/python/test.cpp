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

#define CheckValueInt(value, expected)      UT(value.Int()) == UT(expected)

#define CheckValueFloat(value, expected)    UT(value.Float()) == UT(expected);

#define CheckValueString(value, expected)   do { \
    std::string s = value.Str(); \
    UT(s.c_str()) == UT_CSTR(expected); \
} while (false)

UT_TEST("Variables")
{
    py::Interpreter interpreter;
    {
        py::BlockingSection bs;
    }
    {
        py::ThreadLock lock;
    }

    {
        py::ObjectDict locals = py::ObjectDict::New();
        py::Object res = py::Run(
            "i = 237\n"
            "f = 2.5\n"
            "s = 'test string'",
            locals);
        UT(res.IsNone()) == UT_TRUE;
        CheckValueInt(locals["i"], 237);
        CheckValueFloat(locals["f"], 2.5);
        CheckValueString(locals["s"], "test string");
    }
}
UT_TEST_END
