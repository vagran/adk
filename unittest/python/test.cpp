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
class TestClass: public py::ExposedClassBase {
private:
public:
    long base;
    static bool constrCalled, destrCalled;

    TestClass(py::Object args, py::Object kwArgs): base(0)
    {
        constrCalled = true;
    }

    virtual
    ~TestClass()
    {
        destrCalled = true;
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

bool TestClass::constrCalled, TestClass::destrCalled;

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
    py::Object res = py::Run(
        "import test_module\n"
        "mod_help = test_module.__doc__\n"
        "result = test_module.TestFuncSum(200, 37)\n"
        "func_help = test_module.TestFuncSum.__doc__\n"
        "\n"
        "class_help = test_module.TestClass.__doc__\n"
        "obj = test_module.TestClass(300)\n"
        "obj_hash = hash(obj)\n"
        "obj_str = str(obj)\n"
        "obj_repr = repr(obj)\n"
        "obj_call = obj(10, 15)\n"
        "meth_help = test_module.TestClass.TestMethod.__doc__\n"
        "meth_call = obj.TestMethod(42)\n"
        "meth2_call = obj.TestMethodNoArgs()\n",
        locals);
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

    res = locals["obj"](py::Object(30), py::Object(40));
    CheckValueInt(res, 370);
}
UT_TEST_END
