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
/* Properties::Node class. */

Properties::Node::Node(std::string *name, bool isItem, Node *parent):
    _isItem(isItem), _name(name), _parent(parent)
{}

Properties::Node::~Node()
{}

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
    if (_node->_dispName.empty()) {
        return _node->Name();
    }
    return _node->_dispName;
}

std::string
Properties::Category::Description() const
{
    ASSERT(_node);
    return _node->_description;
}

Properties::Category::operator bool() const
{
    return _node != nullptr;
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
    if (_node->_dispName.empty()) {
        return _node->Name();
    }
    return _node->_dispName;
}

std::string
Properties::Item::Description() const
{
    ASSERT(_node);
    return _node->_description;
}

std::string
Properties::Item::Units() const
{
    ASSERT(_node);
    return _node->_units;
}

Properties::Item::operator bool() const
{
    return _node != nullptr;
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
    //XXX
}
