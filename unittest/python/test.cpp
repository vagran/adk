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
        py::Object res = py::Run(ADK_PY_FILE(test_embedding), locals);
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

/* Exposed function test. */
static py::Object
TestFuncSum(py::Object self, py::Object args)
{
    std::vector<py::Object> argsObj = ADK_PY_PARSE_ARGUMENTS(args, 2, 2);
    if (PyErr_Occurred()) {
        return py::Object();
    }
    return py::Object(argsObj[0].Int() + argsObj[1].Int());
}

/* Exposed class test. */
class TestClass: public py::ExposedClassBase<TestClass> {
private:
public:
    long base;

    TestClass(py::Object args, py::Object kwArgs): base(0)
    {
    }

    virtual
    ~TestClass()
    {
    }

    /* Some overridden standard methods. */

    int
    Init(py::Object args, py::Object kwArgs)
    {
        std::vector<py::Object> argsObj = ADK_PY_PARSE_ARGUMENTS(args, 1, 1);
        if (PyErr_Occurred()) {
            return -1;
        }
        base = argsObj[0].Int();
        return 0;
    }

    py::Object
    Repr()
    {
        std::stringstream ss;
        ss << "TestClass::Repr " << base;
        return py::Object(ss.str().c_str());
    }

    py::Object
    Str()
    {
        std::stringstream ss;
        ss << "TestClass::Str " << base;
        return py::Object(ss.str().c_str());
    }

    Py_hash_t
    Hash()
    {
        return base;
    }

    /* Calling operator automatically exposed to Python as '__call__' method. */
    py::Object
    operator()(py::Object args, py::Object kwArgs)
    {
        std::vector<py::Object> argsObj = ADK_PY_PARSE_ARGUMENTS(args, 2, 2);
        if (PyErr_Occurred()) {
            return py::Object();
        }
        return py::Object(base + argsObj[0].Int() + argsObj[1].Int());
    }

    py::Object
    TestMethod(py::Object args)
    {
        std::vector<py::Object> argsObj = ADK_PY_PARSE_ARGUMENTS(args, 1, 1);
        if (PyErr_Occurred()) {
            return py::Object();
        }
        return py::Object(argsObj[0].Int() + base);
    }

    py::Object
    TestMethodNoArgs()
    {
        return py::Object(base);
    }
};

/* Exposed module description. */
ADK_PYTHON_MODULE(test_module)
{
    Doc("Sample test module");
    DefFunc<TestFuncSum>("TestFuncSum", "Sample test function");
    DefClass<TestClass>("TestClass", "Sample test class").
        DefMethod<&TestClass::TestMethod>("TestMethod", "Sample test method").
        DefMethod<&TestClass::TestMethodNoArgs>("TestMethodNoArgs");
}

UT_TEST("Extension by C++")
{
    py::Interpreter interpreter;
    py::ObjectDict locals = py::ObjectDict::New();
    py::Object res = py::Run(ADK_PY_FILE(test_extending), locals);
    UT(res.IsNone()) == UT_TRUE;
    CheckValueInt(locals["result"], 237);
    CheckValueString(locals["mod_help"], "Sample test module");
    CheckValueString(locals["func_help"], "Sample test function");
    CheckValueString(locals["class_help"], "Sample test class");
    CheckValueInt(locals["obj_hash"], 300);
    CheckValueString(locals["obj_str"], "TestClass::Str 300");
    CheckValueString(locals["obj_repr"], "TestClass::Repr 300");
    CheckValueInt(locals["obj_call"], 325);
    CheckValueString(locals["meth_help"], "Sample test method");
    CheckValueInt(locals["meth_call"], 342);
    CheckValueInt(locals["meth2_call"], 300);
    /* Call object from C++ via Python layer (C++ => Python => C++)*/
    res = locals["obj"](py::Object(30), py::Object(40));
    CheckValueInt(res, 370);
}
