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

/* ****************************************************************************/
/* Various helpers and utilities. */

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

/* ****************************************************************************/

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

    class Item;
    class Category;

    /* ************************************************************************/
    /** Stored value wrapper. The value has dynamic type which is defined when
     * the value is assigned.
     */
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
            return internal::PropValueGet<typename internal::PropValueNearestTypeGet<T>::Type>::
                Get(*this);
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
    /* ************************************************************************/

    /** Parsed node path. */
    class Path {
    public:
        static constexpr size_t npos = static_cast<size_t>(-1);

        /** Construct path from string representation. Path components are
         * separated using the specified separator character. The path may be
         * empty. Empty components (two consequential separator characters) are
         * discarded. Leading or trailing separator does not affect the path.
         * Separator character can be escaped by a backslash. A backslash
         * character itself can be escaped by a backslash. Invalid escape
         * sequence treated as normal characters sequence (i.e. backslash
         * character is preserved).
         */
        Path(const std::string &path, char separator = '/');
        Path(const char *path);
        Path() = default;
        Path(const Path &) = default;
        Path(Path &&) = default;

        Path &
        operator =(const Path &) = default;
        Path &
        operator =(Path &&) = default;

        /** Number of components. */
        size_t
        Size() const;

        /** Check if path is not empty. */
        operator bool () const;

        /** Get path component with the specified index. */
        std::string
        operator[](size_t idx) const &;
        std::string
        operator[](size_t idx) &&;

        /** Concatenate two paths. */
        Path
        operator+(const Path &path) const &;
        Path
        operator+(const Path &path) &&;

        /** Append another path. */
        Path &
        operator +=(const Path &path);

        /** Get string representation for the path. */
        std::string
        Str(char separator = '/') const;

        /** Check if the provided path has common prefix with this path.
         * @return number of components in common prefix. Zero if the paths
         * does not have common prefix.
         */
        size_t
        HasCommonPrefix(const Path &path) const;

        /** Check if this path is prefix for the provided path. */
        bool
        IsPrefixFor(const Path &path) const;

        /** Get sub path.
         *
         * @param start Start position of the subpath.
         * @param count Number of components to extract. Use npos value to
         *      specify subpath till the end of the path.
         * @return Extracted subpath.
         */
        Path
        SubPath(size_t start, size_t count = npos) const &;
        Path
        SubPath(size_t start, size_t count = npos) &&;

    private:
        std::vector<std::string> _components;
    };
    /* ************************************************************************/

private:
    class ItemNode;
    class CategoryNode;

    /** Represents node in the properties tree. */
    class Node {
    public:
        typedef std::shared_ptr<Node> Ptr;

        virtual
        ~Node();

        ItemNode &
        Item();

        CategoryNode &
        Category();

        std::string &
        Name() const;

    protected:
        Node(std::string *name, bool isItem, Node *parent = nullptr);

        /** Indicates whether it is item or category. */
        bool _isItem;
        /** Internal name. */
        std::string *_name;
        /** Parent node, nullptr for root. */
        Node *_parent;
    };

    class ItemNode: public Node {

    private:
        friend class Item;

        //XXX min/maxValue, maxLen should attach built-in validators

        /** Current value. */
        Value _value;
        /** Display name, empty if not specified. */
        std::string _dispName,
        /** Description text. */
                    _description,
        /** Units string, empty if no units specified. */
                    _units;
    };

    class CategoryNode: public Node {
    public:
        /** Get child node by path. nullptr is returned if the node not found. */
        virtual Node *
        Find(const std::string &path);

    private:
        friend class Category;

        /** Display name, empty if not specified. */
        std::string _dispName,
        /** Description text. */
                    _description;
        /** Child nodes. */
        std::map<std::string, Node::Ptr> _children;
    };

public:
    /** Represents item handle exposed to user. */
    class Item {
    public:
        /** Get value type. */
        Value::Type
        Type() const;

        /** Get value. */
        Value
        Val() const;

        /** Get internal name. */
        std::string
        Name() const;

        /** Get string to be used as display name. */
        std::string
        DispName() const;

        /** Get description string. */
        std::string
        Description() const;

        /** Get units string. */
        std::string
        Units() const;

        /** Check if the handle is not empty. */
        operator bool() const;

    private:
        ItemNode *_node;
    };

    /** Represents category handle exposed to user. */
    class Category {
        /** Get internal name. */
        std::string
        Name() const;

        /** Get string to be used as display name. */
        std::string
        DispName() const;

        /** Get description string. */
        std::string
        Description() const;

        /** Check if the handle is not empty. */
        operator bool() const;

    private:
        CategoryNode *_node;
    };

    /** Each sheet modification is done in some transaction context. */
    class Transaction {
    public:
        Transaction(Transaction &&);
        Transaction(const Transaction &) = delete;

        ~Transaction();
    };

    /** Create empty properties. */
    Properties();

    /** Create properties based on the provided XML document. */
    Properties(const Xml &xml);

    /** Clear all the content. All handles are invalidated. */
    void
    Clear();

    /** Replaces all the content with new content from the specified XML
      * document.
      */
    void
    Load(const Xml &xml);

private:
    /** Root category. Its display name contains optional title for the whole
     * properties sheet. Description corresponds to the whole properties sheet
     * optional description.
     */
    Category _root;
};

} /* namespace adk */

#endif /* PROPERTIES_H_ */
