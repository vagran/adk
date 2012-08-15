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
            "s = 'test string'\n"
            "\n"
            "def TestFunc(a, b):\n"
            "   return a + b\n"
            "\n"
            "class TestClass:\n"
            "   prop = 10\n"
            "\n"
            "   def __init__(self, base):\n"
            "       self.base = base\n"
            "   def Sum(self, x):\n"
            "       return self.base + x\n"
            "\n"
            "testObj = TestClass(20)\n",
            locals);
        UT(res.IsNone()) == UT_TRUE;
        CheckValueInt(locals["i"], 237);
        CheckValueFloat(locals["f"], 2.5);
        CheckValueString(locals["s"], "test string");
        /* Test function calling. */
        res = locals["TestFunc"](py::Object(5), py::Object(6));
        CheckValueInt(res, 11);
        /* Test object method calling. */
        res = locals["testObj"].CallMethod("Sum", py::Object(7));
        CheckValueInt(res, 27);
    }
}
UT_TEST_END

static PyObject *
TestFuncSum(PyObject *self, PyObject *args)
{
    return py::Object(237).Steal();
}

ADK_PYTHON_MODULE(test_module)
{
    Doc("Sample test module");
    DefFunc("TestFuncSum", TestFuncSum);
    ADK_INFO("xxx");
}

UT_TEST("Extension by C++")
{
    py::Interpreter interpreter;

}
UT_TEST_END
