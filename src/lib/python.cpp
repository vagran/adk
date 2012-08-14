/* /ADK/src/lib/python.cpp
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file python.cpp
 * Embedded Python support implementation.
 */

#include <adk.h>

using namespace adk;
using namespace py;

Object
py::Run(const std::string &s, int start, Object globals, Object locals,
    PyCompilerFlags *flags)
{
    Object result(PyRun_StringFlags(s.c_str(), start,
                                    globals.Get(), locals.Get(), flags));
    if (!result) {
        /* Exception occurred. */
        Exception exc = Exception::Fetch();
        ADK_INFO("exc:\n%s", exc.Describe().c_str());
        //XXX
    }
    return result;
}
