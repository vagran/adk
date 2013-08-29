/* /ADK/include/adk/properties.h
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file properties.h
 * Properties are helper layer for managing small structured data in form of
 * key, value and some additional attributes. They are suitable for storing and
 * accessing configuration, some program objects properties exposed to the user
 * interface etc. They can be loaded/stored from XML, dynamically displayed in
 * GTK+ windows with a possibility to be edited by user, easily accessed via
 * simple API. Each represented entry can have validator attached. Default
 * validators check the correspondence of the value to one of several built-in
 * types. It is possible to subscribe for value change notifications via signal
 * associated with each entry. Transactions are supported which allow to
 * validate one atomic batch of changes.
 */

#ifndef PROPERTIES_H_
#define PROPERTIES_H_

namespace adk {

namespace internal {

/** Helper for finding nearest matching type for properties value. */
template <typename T>
struct PropValueNearestType {
    /* Integer by default. */
    typedef long Type;
};

template <>
struct PropValueNearestType<double> {
    typedef double Type;
};

template <>
struct PropValueNearestType<float> {
    typedef double Type;
};

template <>
struct PropValueNearestType<bool> {
    typedef bool Type;
};

template <>
struct PropValueNearestType<std::string> {
    typedef std::string Type;
};

template <>
struct PropValueNearestType<const char *> {
    typedef std::string &&Type;
};

} /* namespace internal */

class Properties {
public:
    /** Base exception class for all properties exceptions. */
    ADK_DEFINE_EXCEPTION(Exception);
    /** Document parsing exception. Any kind of inconsistency with schema or
     * data types causes this exception (e.g. string cannot be converted to
     * number, misplaced tag or attribute).
     */
    ADK_DEFINE_DERIVED_EXCEPTION(ParseException, Exception);
    /** Value validation exception. Raised either by built-in or custom user
     * validators.
     */
    ADK_DEFINE_DERIVED_EXCEPTION(ValidationException, Exception);

    /** Stored value wrapper. */
    class Value {
    public:
        enum class Type {
            NONE,
            INTEGER,
            FLOAT,
            BOOLEAN,
            STRING
        };

        Value(Type type = Type::NONE);

        Value(long i);
        Value(double f);
        Value(bool b);
        Value(const std::string &s);
        Value(std::string &&s);

        template <typename T>
        Value(T value):
            Value(static_cast<typename internal::PropValueNearestType<T>::Type>(value))
        {}

        Value(const Value &value);
        Value(Value &&value);

        ~Value();

        Type
        GetType() const
        {
            return _type;
        }

        Value &
        operator =(long value);
        Value &
        operator =(double value);
        Value &
        operator =(bool value);
        Value &
        operator =(const std::string &value);
        Value &
        operator =(std::string &&value);

        template <typename T>
        T
        Get() const
        {
            return static_cast<typename internal::PropValueNearestType<T>::Type>(*this);
        }

        operator long() const;
        operator double() const;
        operator bool() const;
        operator std::string() const;

        template <typename T>
        operator T() const
        {
            return Get<T>();
        }

    private:
        Type _type;

        union {
            long i;
            double f;
            bool b;
            std::string *s;
        } _value;
    };

    /** Represents category handle exposed to user. */
    class Category {

    };

    /** Represents item handle exposed to user. */
    class Item {

    };

    /** Create empty properties. */
    Properties();

    /** Create properties based on the provided XML document. */
    Properties(Xml &xml);
};

} /* namespace adk */

#endif /* PROPERTIES_H_ */
