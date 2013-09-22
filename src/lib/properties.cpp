/* /ADK/src/lib/properties.cpp
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#include <adk.h>

using namespace adk;

/* ****************************************************************************/
/* Properties::Value class. */

Properties::Value::Value(Type type):
    _type(type)
{
    if (type == Type::STRING) {
        _value.s = new std::string();
    }
}

Properties::Value::~Value()
{
    if (_type == Type::STRING) {
        ASSERT(_value.s);
        delete _value.s;
    }
}

Properties::Value::Value(long i):
    _type(Type::INTEGER)
{
    _value.i = i;
}

Properties::Value::Value(double f):
    _type(Type::FLOAT)
{
    _value.f = f;
}

Properties::Value::Value(bool b):
    _type(Type::BOOLEAN)
{
    _value.b = b;
}

Properties::Value::Value(const std::string &s):
    _type(Type::STRING)
{
    _value.s = new std::string(s);
}

Properties::Value::Value(std::string &&s):
    _type(Type::STRING)
{
    _value.s = new std::string(std::move(s));
}

Properties::Value::Value(const Value &value):
    _type(value._type)
{
    if (_type == Type::STRING) {
        _value.s = new std::string(*value._value.s);
    } else if (_type != Type::NONE) {
        _value = value._value;
    }
}

Properties::Value::Value(Value &&value):
    _type(value._type)
{
    _value = value._value;
    value._type = Type::NONE;
}

long
Properties::Value::GetInteger() const &
{
    ENSURE(_type == Type::INTEGER);
    return _value.i;
}

double
Properties::Value::GetFloat() const &
{
    ENSURE(_type == Type::FLOAT);
    return _value.f;
}

bool
Properties::Value::GetBoolean() const &
{
    ENSURE(_type == Type::BOOLEAN);
    return _value.b;
}

std::string
Properties::Value::GetString() const &
{
    ENSURE(_type == Type::STRING);
    return *_value.s;
}

long
Properties::Value::GetInteger() &&
{
    ENSURE(_type == Type::INTEGER);
    int result = _value.i;
    _type = Type::NONE;
    return result;
}

double
Properties::Value::GetFloat() &&
{
    ENSURE(_type == Type::FLOAT);
    double result = _value.f;
    _type = Type::NONE;
    return result;
}

bool
Properties::Value::GetBoolean() &&
{
    ENSURE(_type == Type::BOOLEAN);
    bool result = _value.i;
    _type = Type::NONE;
    return result;
}

std::string
Properties::Value::GetString() &&
{
    ENSURE(_type == Type::STRING);
    std::string result(std::move(*_value.s));
    delete _value.s;
    _value.s = nullptr;
    _type = Type::NONE;
    return result;
}

Properties::Value &
Properties::Value::SetInteger(long value)
{
    if (_type == Type::STRING) {
        delete _value.s;
    }
    _type = Type::INTEGER;
    _value.i = value;
    return *this;
}

Properties::Value &
Properties::Value::SetFloat(double value)
{
    if (_type == Type::STRING) {
        delete _value.s;
    }
    _type = Type::FLOAT;
    _value.f = value;
    return *this;
}

Properties::Value &
Properties::Value::SetBoolean(bool value)
{
    if (_type == Type::STRING) {
        delete _value.s;
    }
    _type = Type::BOOLEAN;
    _value.b = value;
    return *this;
}

Properties::Value &
Properties::Value::SetString(const std::string &value)
{
    if (_type != Type::STRING) {
        _value.s = new std::string(value);
        _type = Type::STRING;
    } else {
        *_value.s = value;
    }
    return *this;
}

Properties::Value &
Properties::Value::SetString(std::string &&value)
{
    if (_type != Type::STRING) {
        _value.s = new std::string(std::move(value));
        _type = Type::STRING;
    } else {
        *_value.s = std::move(value);
    }
    return *this;
}

Properties::Value &
Properties::Value::operator =(const Value &value)
{
    if (_type == Type::STRING) {
        if (value._type == Type::STRING) {
            *_value.s = *value._value.s;
        } else {
            delete _value.s;
            _value = value._value;
        }
    } else {
        if (value._type == Type::STRING) {
            _value.s = new std::string(*value._value.s);
        } else {
            _value = value._value;
        }
    }
    _type = value._type;
    return *this;
}

Properties::Value &
Properties::Value::operator =(Value &&value)
{
    if (_type == Type::STRING) {
        if (value._type != Type::STRING) {
            delete _value.s;
            _value = value._value;
        } else {
            *_value.s = std::move(*value._value.s);
            delete value._value.s;
        }
    } else {
        _value = value._value;
    }
    _type = value._type;
    value._type = Type::NONE;
    return *this;
}

/* ****************************************************************************/
/* Properties::Path class. */

Properties::Path::Path(const std::string &path, char separator)
{
    size_t idx = 0;
    while (idx < path.size()) {
        std::string component;
        bool escape = false;
        for (; idx < path.size(); idx++) {
            char c = path[idx];
            if (escape) {
                if (c != separator && c != '\\') {
                    /* Invalid escape. */
                    component += '\\';
                }
                component += c;
                escape = false;
            } else {
                if (c == separator) {
                    idx++;
                    break;
                } else if (c == '\\') {
                    escape = true;
                } else {
                    component += c;
                }
            }
        };
        if (escape) {
            component += '\\';
        }
        if (!component.empty()) {
            _components.emplace_back(std::move(component));
        }
    }
}

Properties::Path::Path(const char *path):
    Path(std::string(path))
{}

size_t
Properties::Path::Size() const
{
    return _components.size();
}

Properties::Path::operator bool () const
{
    return Size() != 0;
}

std::string
Properties::Path::operator[](size_t idx) const &
{
    ASSERT(idx < _components.size());
    return _components[idx];
}

std::string
Properties::Path::operator[](size_t idx) &&
{
    ASSERT(idx < _components.size());
    return std::move(_components[idx]);
}

Properties::Path
Properties::Path::operator+(const Path &path) const &
{
    Path result(*this);
    result += path;
    return result;
}

Properties::Path
Properties::Path::operator+(const Path &path) &&
{
    (*this) += path;
    return std::move(*this);
}

Properties::Path &
Properties::Path::operator +=(const Path &path)
{
    _components.insert(_components.end(),
                       path._components.begin(), path._components.end());
    return *this;
}

size_t
Properties::Path::HasCommonPrefix(const Path &path) const
{
    size_t idx;
    for (idx = 0;
         idx < _components.size() && idx < path._components.size();
         idx++) {

        if (_components[idx] != path._components[idx]) {
            break;
        }
    }
    return idx;
}

bool
Properties::Path::IsPrefixFor(const Path &path) const
{
    return HasCommonPrefix(path) == _components.size();
}

std::string
Properties::Path::Str(char separator) const
{
    std::string result;
    for (const std::string &comp: _components) {
        if (!result.empty()) {
            result += separator;
        }
        for (char c: comp) {
            if (c == separator || c == '\\') {
                result += '\\';
            }
            result += c;
        }
    }
    return result;
}

Properties::Path
Properties::Path::SubPath(size_t start, size_t count) const &
{
    ASSERT(start <= _components.size());
    ASSERT(count == npos || start + count <= _components.size());
    Path result;
    result._components.insert(
        result._components.begin(),
        _components.begin() + start,
        count == npos ? _components.end() : _components.begin() + start + count);
    return result;
}

Properties::Path
Properties::Path::SubPath(size_t start, size_t count) &&
{
    ASSERT(start <= _components.size());
    ASSERT(count == npos || start + count <= _components.size());
    if (start != 0) {
        _components.erase(_components.begin(), _components.begin() + start);
    }
    if (count != npos) {
        _components.resize(count);
    }
    return std::move(*this);
}

/* ****************************************************************************/
/* Properties::Node class. */

Properties::Node::Node(std::string *name, bool isItem, Node *parent,
                       Transaction *trans):
    _isItem(isItem), _name(name), _parent(parent), _transaction(trans)
{}

Properties::Node::~Node()
{}

bool
Properties::Node::IsItem() const
{
    return _isItem;
}

Properties::ItemNode &
Properties::Node::Item()
{
    ASSERT(_isItem);
    return reinterpret_cast<ItemNode &>(*this);
}

Properties::CategoryNode &
Properties::Node::Category()
{
    ASSERT(!_isItem);
    return reinterpret_cast<CategoryNode &>(*this);
}

std::string &
Properties::Node::Name() const
{
    return *_name;
}

/* ****************************************************************************/
/* Properties::Category class. */

std::string
Properties::Category::Name() const
{
    ASSERT(_node);
    return _node->Name();
}

std::string
Properties::Category::DispName() const
{
    ASSERT(_node);
    if (!_node->_dispName) {
        return _node->Name();
    }
    return *_node->_dispName;
}

std::string
Properties::Category::Description() const
{
    ASSERT(_node);
    return _node->_description ? *_node->_description : std::string();
}

Properties::Category::operator bool() const
{
    return _node != nullptr;
}

/* ****************************************************************************/
/* Properties::Category::Options class. */

Properties::Category::Options &
Properties::Category::Options::DispName(Optional<std::string> dispName)
{
    this->dispName = dispName;
    return *this;
}

Properties::Category::Options &
Properties::Category::Options::Description(Optional<std::string> description)
{
    this->description = description;
    return *this;
}

/* ****************************************************************************/
/* Properties::Item class. */

Properties::Value::Type
Properties::Item::Type() const
{
    ASSERT(_node);
    return _node->_value.GetType();
}

Properties::Value
Properties::Item::Val() const
{
    ASSERT(_node);
    return _node->_value;
}

std::string
Properties::Item::Name() const
{
    ASSERT(_node);
    return _node->Name();
}

std::string
Properties::Item::DispName() const
{
    ASSERT(_node);
    if (!_node->_dispName) {
        return _node->Name();
    }
    return *_node->_dispName;
}

std::string
Properties::Item::Description() const
{
    ASSERT(_node);
    return _node->_description ? *_node->_description : std::string();
}

std::string
Properties::Item::Units() const
{
    ASSERT(_node);
    return _node->_units ? *_node->_units : std::string();
}

Properties::Item::operator bool() const
{
    return _node != nullptr;
}

/* ****************************************************************************/
/* Properties::Item::Options class. */

Properties::Item::Options &
Properties::Item::Options::DispName(Optional<std::string> dispName)
{
    this->dispName = dispName;
    return *this;
}

Properties::Item::Options &
Properties::Item::Options::Description(Optional<std::string> description)
{
    this->description = description;
    return *this;
}

Properties::Item::Options &
Properties::Item::Options::Units(Optional<std::string> units)
{
    this->description = units;
    return *this;
}

/* ****************************************************************************/
/* Properties::Transaction class. */

Properties::Transaction::Transaction(Transaction &&trans __UNUSED)
{
    //XXX
}

Properties::Transaction::~Transaction()
{
    //XXX
}

void
Properties::Transaction::Commit()
{
    //XXX
}

void
Properties::Transaction::Cancel()
{
    //XXX
}

Properties::Category
Properties::Transaction::AddCategory(const Path &path __UNUSED,
                                     const Category::Options &options __UNUSED)
{
    //XXX
    return Category();
}

Properties::Item
Properties::Transaction::AddItem(const Path &path __UNUSED, const Value &value __UNUSED,
                                 const Item::Options &options __UNUSED)
{
    //XXX
    return Item();
}

Properties::Item
Properties::Transaction::AddItem(const Path &path __UNUSED, Value &&value __UNUSED,
                                 const Item::Options &options __UNUSED)
{
    //XXX
    return Item();
}


/* ****************************************************************************/
/* Properties class. */

Properties::Properties()
{

}

Properties::Properties(const Xml &xml)
{
    Load(xml);
}

void
Properties::Clear()
{
    //XXX
}

void
Properties::Load(const Xml &xml __UNUSED)
{
    Transaction::Ptr trans = OpenTransaction();
    //XXX
}

Properties::Transaction::Ptr
Properties::OpenTransaction()
{
    //XXX
    return nullptr;
}
