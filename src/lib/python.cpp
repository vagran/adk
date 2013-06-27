/* /ADK/src/lib/python.cpp
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
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

#endif /* ADK_USE_PYTHON */
