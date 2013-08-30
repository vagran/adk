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

/** Helper for finding nearest matching type for properties value when assigning. */
template <typename T>
struct PropValueNearestTypeSet {
    /* Integer by default. */
    typedef long Type;
};

template <>
struct PropValueNearestTypeSet<double> {
    typedef double Type;
};

template <>
struct PropValueNearestTypeSet<float> {
    typedef double Type;
};

template <>
struct PropValueNearestTypeSet<bool> {
    typedef bool Type;
};

template <>
struct PropValueNearestTypeSet<std::string> {
    typedef std::string &&Type;
};

template <>
struct PropValueNearestTypeSet<std::string &> {
    typedef const std::string &Type;
};

template <>
struct PropValueNearestTypeSet<const char *> {
    typedef std::string &&Type;
};

template <size_t size>
struct PropValueNearestTypeSet<const char (&)[size]> {
    typedef std::string &&Type;
};

/** Helper for finding nearest matching type for properties value when getting value. */
template <typename T>
struct PropValueNearestTypeGet {
    /* Integer by default. */
    typedef long Type;
};

template <>
struct PropValueNearestTypeGet<double> {
    typedef double Type;
};

template <>
struct PropValueNearestTypeGet<float> {
    typedef double Type;
};

template <>
struct PropValueNearestTypeGet<bool> {
    typedef bool Type;
};

template <>
struct PropValueNearestTypeGet<std::string> {
    typedef std::string Type;
};

template <>
struct PropValueNearestTypeGet<const char *> {
    typedef const char *Type;
};

/** Helper class for getting value of necessary type. */
template <typename T>
class PropValueGet;

template <>
class PropValueGet<long> {
public:
    template <class Props>
    static long
    Get(Props &&prop)
    {
        return std::forward<Props>(prop).GetInteger();
    }
};

template <>
class PropValueGet<double> {
public:
    template <class Props>
    static double
    Get(Props &&prop)
    {
        return std::forward<Props>(prop).GetFloat();
    }
};

template <>
class PropValueGet<bool> {
public:
    template <class Props>
    static bool
    Get(Props &&prop)
    {
        return std::forward<Props>(prop).GetBoolean();
    }
};

template <>
class PropValueGet<std::string> {
public:
    template <class Props>
    static std::string
    Get(Props &&prop)
    {
        return std::forward<Props>(prop).GetString();
    }
};

template <>
class PropValueGet<const char *> {
public:
    template <class Props>
    static const char *
    Get(Props &&prop)
    {
        return std::forward<Props>(prop).GetString().c_str();
    }
};

/** Helper class for setting value of necessary type. */
template <typename T>
class PropValueSet;

template <>
class PropValueSet<long> {
public:
    template <class Props>
    static Props &
    Set(Props &prop, long value)
    {
        return prop.SetInteger(value);
    }
};

template <>
class PropValueSet<double> {
public:
    template <class Props>
    static Props &
    Set(Props &prop, double value)
    {
        return prop.SetFloat(value);
    }
};

template <>
class PropValueSet<bool> {
public:
    template <class Props>
    static Props &
    Set(Props &prop, bool value)
    {
        return prop.SetBoolean(value);
    }
};

template <>
class PropValueSet<const std::string &> {
public:
    template <class Props>
    static Props &
    Set(Props &prop, const std::string &value)
    {
        return prop.SetString(value);
    }
};

template <>
class PropValueSet<std::string &&> {
public:
    template <class Props>
    static Props &
    Set(Props &prop, std::string &&value)
    {
        return prop.SetString(std::move(value));
    }
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
            Value(static_cast<typename internal::PropValueNearestTypeSet<T>::Type>(value))
        {}

        Value(const Value &value);
        Value(Value &&value);

        ~Value();

        Type
        GetType() const
        {
            return _type;
        }

        /* Obtain copy of the value. */
        long
        GetInteger() const &;
        double
        GetFloat() const &;
        bool
        GetBoolean() const &;
        std::string
        GetString() const &;

        /* Take value away. The value is none after that. Especially useful for
         * taking string value - data are not copied in such case.
         */
        long
        GetInteger() &&;
        double
        GetFloat() &&;
        bool
        GetBoolean() &&;
        std::string
        GetString() &&;

        template <typename T>
        T
        Get() const &
        {
            return internal::PropValueGet<typename internal::PropValueNearestTypeGet<T>::Type>::Get(*this);
        }

        template <typename T>
        T
        Get() &&
        {
            return internal::PropValueGet<typename internal::PropValueNearestTypeGet<T>::Type>::
                Get(std::move(*this));
        }

        template <typename T>
        operator T() const &
        {
            return Get<T>();
        }

        template <typename T>
        operator T() &&
        {
            return std::move(*this).Get<T>();
        }

        /** Check if value is none. */
        bool
        IsNone() const
        {
            return _type == Type::NONE;
        }

        Value &
        SetInteger(long value);
        Value &
        SetFloat(double value);
        Value &
        SetBoolean(bool value);
        Value &
        SetString(const std::string &value);
        Value &
        SetString(std::string &&value);

        template <typename T>
        Value &
        Set(T &&value)
        {
            return internal::PropValueSet<typename internal::PropValueNearestTypeSet<T>::Type>::
                Set(*this, std::forward<T>(value));
        }

        template <typename T>
        Value &
        operator =(T &&value)
        {
            return internal::PropValueSet<typename internal::PropValueNearestTypeSet<T>::Type>::
                Set(*this, std::forward<T>(value));
        }

        Value &
        operator =(const Value &value);
        Value &
        operator =(Value &&value);

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
