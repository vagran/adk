/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
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

/* Xml::AttributeNode class. */

Xml::AttributeNode::AttributeNode(ElementNode &e, NameId nameId,
                                  const std::string &value):
    _element(e), _nameId(nameId), _value(value)
{}

std::string
Xml::AttributeNode::Name() const
{
    return _element._doc._GetName(_nameId);
}

void
Xml::AttributeNode::SetValue(const std::string &value)
{
    ASSERT(_nameId);
    _value = value;
}

Xml::AttributeNode *
Xml::AttributeNode::Next() const
{
    auto it = _element._attrs.find(_nameId);
    if (it == _element._attrs.end()) {
        return nullptr;
    }
    it++;
    if (it == _element._attrs.end()) {
        return nullptr;
    }
    return it->second.get();
}

void
Xml::AttributeNode::Remove()
{
    auto it = _element._attrs.find(_nameId);
    if (it == _element._attrs.end()) {
        return;
    }
    _element._attrs.erase(it);
}

/* Xml::ElementNode class. */

Xml::AttributeNode *
Xml::ElementNode::_SetAttribute(NameId nameId, const std::string &value)
{
    auto it = _attrs.find(nameId);
    if (it == _attrs.end()) {
        it = _attrs.emplace(nameId,
                            AttributeNode::Ptr(new AttributeNode(*this, nameId, value))).first;
    } else {
        it->second->SetValue(value);
    }
    return it->second.get();
}

Xml::AttributeNode *
Xml::ElementNode::SetAttribute(const std::string &name, const std::string &value)
{
    return _SetAttribute(_doc._AddName(name), value);
}

Xml::AttributeNode *
Xml::ElementNode::FirstAttribute() const
{
    auto it = _attrs.begin();
    if (it == _attrs.end()) {
        return nullptr;
    }
    return it->second.get();
}

void
Xml::ElementNode::_AddCharData(const std::string &data)
{
    _value += data;
}

Xml::AttributeNode *
Xml::ElementNode::Attribute(const std::string &name) const
{
    Xml::NameId nid = _doc._GetNameId(name);
    if (!nid) {
        return nullptr;
    }
    auto it = _attrs.find(nid);
    if (it == _attrs.end()) {
        return nullptr;
    }
    return it->second.get();
}

void
Xml::ElementNode::_AddChild(ElementNode::Ptr &&e)
{
    auto it = _children.find(e->_nameId);
    if (it == _children.end()) {
        it = _children.emplace(e->_nameId, SiblingList()).first;
    }
    e->_parent = this;
    SiblingList &sl = it->second;
    sl.list.push_back(std::move(e));
}

Xml::ElementNode *
Xml::ElementNode::Child(const std::string &name) const
{
    decltype(_children.begin()) it;
    if (name.empty()) {
        it = _children.begin();
    } else {
        Xml::NameId nid = _doc._GetNameId(name);
        if (!nid) {
            return nullptr;
        }
        it = _children.find(nid);
    }
    if (it == _children.end()) {
        return nullptr;
    }
    if (it->second.list.size() == 0) {
        return nullptr;
    }
    return it->second.list.front().get();
}

Xml::ElementNode *
Xml::ElementNode::NextSibling(const std::string &name) const
{
    if (!_parent) {
        return nullptr;
    }
    NameId nid;
    if (name.empty()) {
        nid = 0;
    } else {
        nid = _doc._GetNameId(name);
        if (!nid) {
            return nullptr;
        }
    }
    auto it = _parent->_children.find(_nameId);
    if (it == _parent->_children.end()) {
        return nullptr;
    }
    if (!nid || nid == _nameId) {
        auto siblIt = it->second.list.begin();
        while (siblIt->get() != this) {
            siblIt++;
            if (siblIt == it->second.list.end()) {
                return nullptr;
            }
        }
        siblIt++;
        if (siblIt != it->second.list.end()) {
            return siblIt->get();
        }
        if (nid) {
            return nullptr;
        }
        /* Any next sibling element. */
        while (true) {
            it++;
            if (it == _parent->_children.end()) {
                return nullptr;
            }
            auto siblIt = it->second.list.begin();
            if (siblIt == it->second.list.end()) {
                continue;
            }
            return siblIt->get();
        }
    }
    if (nid < _nameId) {
        return nullptr;
    }
    it = _parent->_children.find(_nameId);
    if (it == _parent->_children.end()) {
        return nullptr;
    }
    auto siblIt = it->second.list.begin();
    if (siblIt == it->second.list.end()) {
        return nullptr;
    }
    return siblIt->get();
}

void
Xml::ElementNode::Remove()
{
    if (!_parent) {
        return;
    }
    auto it = _parent->_children.find(_nameId);
    if (it == _parent->_children.end()) {
        return;
    }
    it->second.list.remove_if([this](ElementNode::Ptr &e){ return e.get() == this; });
}

Xml::ElementNode *
Xml::ElementNode::AddChild(const std::string &name)
{
    NameId nid = _doc._AddName(name);
    const char *attrs = nullptr;
    ElementNode::Ptr e = _doc._CreateElement(nid, &attrs);
    ElementNode *ePtr = e.get();
    _AddChild(std::move(e));
    return ePtr;
}

/* Xml class. */

Xml::Xml()
{
}

Xml::~Xml()
{
    Clear();
}

Xml &
Xml::Load(const char *buf, size_t size)
{
    if (size == std::string::npos) {
        return Load(std::string(buf));
    } else {
        return Load(std::string(buf, size));
    }
}

Xml &
Xml::Load(const std::string &buf)
{
    std::istringstream ss(buf);
    return Load(ss);
}

Xml &
Xml::Load(std::istream &stream)
{
    Clear();
    _CreateParser();
    char buf[1024];
    size_t size;
    do {
        stream.read(buf, sizeof(buf));
        size = stream.gcount();
        if (!size) {
            break;
        }
        XML_Status status = XML_Parse(_parser, buf, size, size < sizeof(buf));
        if (status != XML_STATUS_OK) {
            _CreateParseException();
        }
    } while (size == sizeof(buf));
    return *this;
}

void
Xml::_SaveIndentation(int indentation, std::ostream &stream)
{
    while (indentation--) {
        stream << "  ";
    }
}

void
Xml::_SaveElement(Element e, int indentation, std::ostream &stream) const
{
    _SaveIndentation(indentation, stream);
    stream << "<" << e.Name();
    for (Attribute attr: e.Attributes()) {
        stream << ' ' << attr.Name() << "=\"" << EscapeEntities(attr.Value()) << '"';
    }
    if (e.Children() || !e.ValueEmpty()) {
        stream << '>';
        if (e.Children()) {
            stream << std::endl;
        }
        for (Element child: e.Children()) {
            _SaveElement(child, indentation + 1, stream);
        }
        std::string value;
        if (!e.ValueEmpty()) {
            value = EscapeEntities(e.Value(), e.Children());
        }
        if (!value.empty()) {
            stream << value;
        } else {
            _SaveIndentation(indentation, stream);
        }
        stream << "</" << e.Name() << '>' << std::endl;
    } else {
        stream << "/>" << std::endl;
    }
}

void
Xml::Save(std::ostream &stream) const
{
    stream << "<?xml version=\"1.0\" encoding=\"utf-8\" ?>" << std::endl;
    _SaveElement(Root(), 0, stream);
}

void
Xml::Save(std::string &str) const
{
    std::ostringstream ss;
    Save(ss);
    str = ss.str();
}

std::string
Xml::EscapeEntities(const std::string &s, bool trimWhitespaces)
{
    bool nonWsSeen = false;
    size_t firstWsPos = std::string::npos;

    std::string result;
    for (int c: s) {
        switch(c) {
        case '<':
            result += "&lt;";
            nonWsSeen = true;
            firstWsPos = std::string::npos;
            break;
        case '>':
            result += "&gt;";
            nonWsSeen = true;
            firstWsPos = std::string::npos;
            break;
        case '"':
            result += "&quot;";
            nonWsSeen = true;
            firstWsPos = std::string::npos;
            break;
        case '\'':
            result += "&apos;";
            nonWsSeen = true;
            firstWsPos = std::string::npos;
            break;
        case '&':
            result += "&amp;";
            nonWsSeen = true;
            firstWsPos = std::string::npos;
            break;
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            if (!trimWhitespaces || nonWsSeen) {
                result += c;
            }
            if (nonWsSeen && firstWsPos == std::string::npos) {
                firstWsPos = result.size() - 1;
            }
            break;
        default:
            result += c;
        }
    }
    if (trimWhitespaces && firstWsPos != std::string::npos) {
        return result.substr(0, firstWsPos);
    }
    return result;
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
    if (_root) {
        _root = nullptr;
    }
    _curElement = nullptr;
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
        ADK_EXCEPTION(Exception, "Invalid ID");
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
Xml::_StartElementHandler(const XML_Char *name, const XML_Char **attrs)
{
    NameId nid = _AddName(name);
    if (!_root) {
        _root = _CreateElement(nid, attrs, _parser);
        _curElement = _root.get();
    } else {
        ElementNode::Ptr e = _CreateElement(nid, attrs, _parser);
        ElementNode *ePtr = e.get();
        _curElement->_AddChild(std::move(e));
        _curElement = ePtr;
    }
}

void
Xml::_EndElementHandler(const XML_Char *name __UNUSED)
{
    ASSERT(_GetNameId(name) == _curElement->_nameId);
    _curElement = _curElement->Parent();
}

Xml::ElementNode::Ptr
Xml::_CreateElement(NameId nameId, const XML_Char **attrs, const Location &loc)
{
    ElementNode::Ptr e = ElementNode::Ptr(new ElementNode(*this, loc, nameId));
    while (*attrs) {
        NameId nid = _AddName(attrs[0]);
        e->_SetAttribute(nid, attrs[1]);
        attrs += 2;
    }
    return e;
}

void
Xml::_CharDataHandler(const XML_Char *s, int len)
{
    _curElement->_AddCharData(std::string(s, len));
}
