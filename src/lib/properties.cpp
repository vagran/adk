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

Properties::Value::operator long() const
{
    ENSURE(_type == Type::INTEGER);
    return _value.i;
}

Properties::Value::operator double() const
{
    ENSURE(_type == Type::FLOAT);
    return _value.f;
}

Properties::Value::operator bool() const
{
    ENSURE(_type == Type::BOOLEAN);
    return _value.b;
}

Properties::Value::operator std::string() const
{
    ENSURE(_type == Type::STRING);
    return *_value.s;
}

/* ****************************************************************************/

Properties::Properties()
{

}

Properties::Properties(Xml &xml __UNUSED)
{

}
