/* This file is a part of ADK library.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file python.cpp
 * Embedded Python support implementation.
 */

#ifdef ADK_USE_PYTHON

#include <adk.h>

using namespace adk;
using namespace py;

std::atomic<int> py::Interpreter::_refCount;

std::string
py::Exception::Describe(PyObject *excType, PyObject *excValue, PyObject *traceback)
{
    if (UNLIKELY(!excType)) {
        return std::string("No exception");
    }
    ObjectModule tbMod("traceback");
    Object formatFunc(tbMod.GetAttr("format_exception"));
    ObjectSequence result(formatFunc(Object(excType, false),
                                     Object(excValue, false),
                                     traceback ? Object(traceback, false) : Object::None()));
    std::string desc;
    for (auto line: result) {
        desc += line.Str();
    }
    return desc;
}

std::string
py::Object::Str() const
{
    ObjectUnicode s(PyObject_Str(_obj));
    if (UNLIKELY(!s)) {
        ADK_PY_CHECK_EXCEPTION();
    }
    return s.GetString();
}

std::string
py::Object::Repr() const
{
    ObjectUnicode s(PyObject_Repr(_obj));
    if (UNLIKELY(!s)) {
        ADK_PY_CHECK_EXCEPTION();
    }
    return s.GetString();
}

Object
py::Object::GetItem(const char *key) const
{
    ObjectUnicode s(key);
    return GetItem(s);
}

Object
py::Run(const std::string &s, Object locals, Object globals, int start,
        PyCompilerFlags *flags)
{
    /* Add 'builtins' dictionary automatically. */
    ObjectDict builtins = ObjectDict::Builtins();
    locals.SetItem("__builtins__", builtins);
    globals.SetItem("__builtins__", builtins);

    Object result(PyRun_StringFlags(s.c_str(), start,
                                    globals.Get(), locals.Get(), flags));
    if (!result) {
        /* Exception occurred. */
        ADK_PY_CHECK_EXCEPTION();
    }
    return result;
}

void
py::internal::ModuleRegistrator::Register(const char *name, InitFunc initFunc)
{
    _moduleDef.m_name = name;
    _moduleDef.m_size = -1;
    _moduleDef.m_methods = &_methods.front();
    PyImport_AppendInittab(name, initFunc);
    //XXX classes
}

void
py::internal::ModuleRegistrator::_AddMethod(const char *name, PyCFunction func,
                                            int flags, const char *doc)
{
    _methods.resize(_methods.size() + 1);
    PyMethodDef *def = &_methods.back() - 1;
    def->ml_name = name;
    def->ml_meth = func;
    def->ml_flags = flags;
    def->ml_doc = doc;
}

std::vector<Object>
adk::py::ParseArguments(Object args, const char *func, const char *file, int line,
                        int maxArgs, int minArgs)
{
    ASSERT(maxArgs == -1 || minArgs == -1 || maxArgs >= minArgs);
    ASSERT(PyTuple_Check(args.Get()));
    Py_ssize_t size = PyTuple_Size(args.Get());
    if (UNLIKELY(minArgs != -1 && size < minArgs)) {
        std::stringstream ss;
        if (func) {
            ss << "[" << file << ":" << line << "] Function '" << func <<
                "()' expects at least " << minArgs << " arguments (" <<
                size << " given)";
        } else {
            ss << "The function expects at least " << minArgs <<
                " arguments (" << size << " given)";
        }
        throw ExpException(ss.str().c_str(), PyExc_TypeError);
    }
    if (UNLIKELY(maxArgs != -1 && size > maxArgs)) {
        std::stringstream ss;
        if (file) {
            ss << "[" << file << ":" << line << "] Function '" << func <<
                "()' expects at most " << maxArgs << " arguments (" <<
                size << " given)";
        } else {
            ss << "The function expects at most " << maxArgs <<
                " arguments (" << size << " given)";
        }
        throw ExpException(ss.str().c_str(), PyExc_TypeError);
    }
    std::vector<Object> result(size);
    for (Py_ssize_t i = 0; i < size; i++) {
        result[i] = Object(PyTuple_GetItem(args.Get(), i), false);
    }
    return result;
}

std::vector<Object>
adk::py::ParseArguments(Object args, int maxArgs, int minArgs)
{
    return ParseArguments(args, nullptr, nullptr, 0, maxArgs, minArgs);
}

#endif /* ADK_USE_PYTHON */
