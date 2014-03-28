/* /ADK/src/lib/exception.cpp
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file exception.cpp
 * Exceptions support code.
 */

#include <adk.h>

using namespace adk;

/** Define string conversion method for trivial types. */
#define __ADK_PARAM_EXC_DEF_STR_METHOD(__type) \
    template <> \
    void \
    ParamException<__type>::_AppendParamStr() \
    { \
        std::stringstream ss; \
        ss << ": " << "[" << _param << "]"; \
        _msg += ss.str(); \
    }

__ADK_PARAM_EXC_DEF_STR_METHOD(bool);
__ADK_PARAM_EXC_DEF_STR_METHOD(char);
__ADK_PARAM_EXC_DEF_STR_METHOD(unsigned char);
__ADK_PARAM_EXC_DEF_STR_METHOD(short);
__ADK_PARAM_EXC_DEF_STR_METHOD(unsigned short);
__ADK_PARAM_EXC_DEF_STR_METHOD(int);
__ADK_PARAM_EXC_DEF_STR_METHOD(unsigned int);
__ADK_PARAM_EXC_DEF_STR_METHOD(long);
__ADK_PARAM_EXC_DEF_STR_METHOD(unsigned long);
__ADK_PARAM_EXC_DEF_STR_METHOD(long long);
__ADK_PARAM_EXC_DEF_STR_METHOD(unsigned long long);
