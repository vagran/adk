/* /ADK/src/lib/xml.cpp
 *
 * This file is a part of 'ADK' project.
 * Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file xml.cpp
 * XML manipulation.
 */

#include <adk.h>

using namespace adk;

namespace {

/** Helper class for binding Xml class methods as XML parser handlers. */
template <typename Result, typename... Args>
class XmlBinder {
public:
    /** Handler type. */
    typedef Result (XMLCALL *Handler_type)(void *, Args...);
    /** Method type. */
    typedef Result (adk::Xml::*Method_type)(Args...);

    template <Method_type method>
    static Result
    Handler(void *userData, Args... args)
    {
        return (reinterpret_cast<Xml *>(userData)->*method)(std::forward<Args>(args)...);
    }
};

/** Specialization for void return type. */
template <typename... Args>
class XmlBinder<void, Args...> {
public:
    /** Handler type. */
    typedef void (XMLCALL *Handler_type)(void *, Args...);
    /** Method type. */
    typedef void (adk::Xml::*Method_type)(Args...);

    template <Method_type method>
    static void
    Handler(void *userData, Args... args)
    {
        (reinterpret_cast<Xml *>(userData)->*method)(std::forward<Args>(args)...);
    }
};

template <typename Result, typename... Args>
XmlBinder<Result, Args...>
_XmlBinderType(Result (adk::Xml::*)(Args...))
{
    return std::declval<XmlBinder<Result, Args...>>();
}

template <typename Method, Method method>
typename decltype(_XmlBinderType(method))::Handler_type
XmlBind()
{
    typedef decltype(_XmlBinderType(method)) Binder_type;
    return Binder_type::template Handler<method>;
}

/** Bind method of Xml class object as parser handler. */
#define XML_BIND(__method) XmlBind<decltype(__method), __method>()

} /* anonymous namespace */

Xml::Xml()
{
}

Xml::~Xml()
{
    Clear();
}

void
Xml::Load(const char *buf, size_t size)
{
    Clear();
    _CreateParser();
    XML_Status status = XML_Parse(_parser, buf, size, true);
    if (status != XML_STATUS_OK) {
        _CreateParseException();
    }
}

void
Xml::_CreateParseException()
{
    XML_Error code = XML_GetErrorCode(_parser);
    const XML_LChar *errMsg = XML_ErrorString(code);
    XML_Size line = XML_GetCurrentLineNumber(_parser);
    XML_Size col = XML_GetCurrentColumnNumber(_parser);
    ADK_EXCEPTION(ParseException, "line " << line << " column " << col <<
                  ": [" << code << "] " << errMsg);
}

void
Xml::Clear()
{
    if (_parser) {
        XML_ParserFree(_parser);
        _parser = nullptr;
    }
    _namesIndex.clear();
    _names.clear();
    _curNameId = 1;
}

void
Xml::_CreateParser()
{
    _parser = XML_ParserCreate(nullptr);
    XML_SetUserData(_parser, this);
    XML_SetStartElementHandler(_parser, XML_BIND(&Xml::_StartElementHandler));
    XML_SetEndElementHandler(_parser, XML_BIND(&Xml::_EndElementHandler));
    XML_SetCharacterDataHandler(_parser, XML_BIND(&Xml::_CharDataHandler));
}

Xml::NameId
Xml::_AddName(const std::string &name)
{
    auto it = _names.find(name);
    if (it != _names.end()) {
        return it->second;
    }
    it = _names.emplace(name, _curNameId++).first;
    _namesIndex.emplace(it->second, it->first);
    return it->second;
}

std::string
Xml::_GetName(NameId id) const
{
    if (!id || id >= _curNameId) {
        ADK_EXCEPTION(Exception, "Invalid id");
    }
    auto it = _namesIndex.find(id);
    ASSERT(it != _namesIndex.end());
    return it->second;
}

Xml::NameId
Xml::_GetNameId(const std::string name) const
{
    auto it = _names.find(name);
    if (it == _names.end()) {
        return 0;
    }
    return it->second;
}

void
Xml::_StartElementHandler(const XML_Char *name, const XML_Char **attrs __UNUSED)
{
    ADK_INFO("start name = %s", name);//XXX
}

void
Xml::_EndElementHandler(const XML_Char *name)
{
    ADK_INFO("end name = %s", name);//XXX
}

void
Xml::_CharDataHandler(const XML_Char *s, int len)
{
    ADK_INFO("data = %s", std::string(s, len).c_str());//XXX
}
