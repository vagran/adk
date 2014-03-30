/* /ADK/include/adk/properties.h
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
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
    /** The requested operation is invalid. */
    ADK_DEFINE_DERIVED_EXCEPTION(InvalidOpException, Exception);

    class Item;
    class Category;
    class Transaction;

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

        /** Get type by its name.
         *
         * @return NONE if type not recognized.
         */
        static Type
        TypeFromString(const std::string &typeStr);

        /** Create value from the provided string.
         *
         * @param type Value type.
         * @param s String with value representation.
         * @return Parsed value.
         * @throws ParseException if the value cannot be parsed from the
         *      provided string.
         */
        static Value
        FromString(Type type, const std::string &s);

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
         *      does not have common prefix.
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

        Path
        Parent() const &;
        Path
        Parent() &&;

        std::string
        First() const &;
        std::string
        First() &&;

        std::string
        Last() const &;
        std::string
        Last() &&;

        bool
        operator ==(const Path &path) const;
        bool
        operator !=(const Path &path) const;

    private:
        std::vector<std::string> _components;
    };
    /* ************************************************************************/

private:
    class ItemNode;
    class CategoryNode;

    /** Represents node in the properties tree. */
    class Node: public std::enable_shared_from_this<Node> {
    public:
        typedef std::shared_ptr<Node> Ptr;

        virtual
        ~Node();

        Ptr
        GetPtr();

        bool
        IsItem() const;

        bool
        IsCategory() const;

        ItemNode &
        Item();

        CategoryNode &
        Category();

        std::string
        Name() const;

        /** Unlink the node from its parent. */
        void
        Unlink();

    protected:
        friend class Transaction;
        friend class Properties;

        Node(bool isItem, Transaction *trans);

        /** Indicates whether it is item or category. */
        bool _isItem;
        /** Internal name. */
        const std::string *_name = nullptr;
        /** Parent node, nullptr for root. */
        Node *_parent = nullptr;

        /** Associated transaction if not yet committed to the sheet. */
        Transaction *_transaction;
    };

    /** Leaf (item) node. */
    class ItemNode: public Node {
    public:
        static Ptr
        Create(Transaction *trans);

        ItemNode(Transaction *trans);

    private:
        friend class Item;
        friend class Transaction;

        //XXX min/maxValue, maxLen should attach built-in validators

        /** Current value. */
        Value _value;
        /** Display name, empty if not specified. */
        Optional<std::string> _dispName,
        /** Description text. */
                              _description,
        /** Units string, empty if no units specified. */
                              _units;
    };

    /** Non-leaf (category) node. */
    class CategoryNode: public Node {
    public:
        static Ptr
        Create(Transaction *trans);

        CategoryNode(Transaction *trans);

        /** Get child node by path. nullptr is returned if the node not found.
         * @param itemInPathFatal Throws InvalidOpException if found leaf item
         *      when searching for a path.
         */
        Ptr
        Find(const Path &path, bool itemInPathFatal = false);

        /** Add new child node. */
        void
        AddChild(const std::string &name, Ptr node);

        /** Unlink child node with the specified name. */
        void
        UnlinkChild(const std::string &name);

    private:
        friend class Category;
        friend class Transaction;

        /** Display name, empty if not specified. */
        Optional<std::string> _dispName,
        /** Description text. */
                              _description;
        /** Child nodes. */
        std::map<std::string, Ptr> _children;
    };

public:
    /** Represents item handle exposed to user. */
    class Item {
    public:

        /** Item creation options. */
        class Options {
        public:
            Optional<std::string> dispName,
                                  description,
                                  units;

            Options &
            DispName(Optional<std::string> dispName);

            Options &
            Description(Optional<std::string> description);

            Options &
            Units(Optional<std::string> units);
        };

        Item(ItemNode *node = nullptr);

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
    public:
        /** Category creation options. */
        class Options {
        public:

            Optional<std::string> dispName,
                                  description;

            Options &
            DispName(Optional<std::string> dispName);

            Options &
            Description(Optional<std::string> description);
        };

        Category(CategoryNode *node = nullptr);

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

    /** Each sheet modification is done in some transaction context. A
     * transaction accumulates modification operations (node change/add/delete).
     * When a transaction is committed the properties tree is locked.
     * */
    class Transaction {
    public:
        typedef std::shared_ptr<Transaction> Ptr;

        static Ptr
        Create(Properties *props);

        Transaction(Transaction &&trans);
        Transaction(const Transaction &) = delete;
        Transaction(Properties *props);

        ~Transaction();

        /** Commit all accumulated operations. Validation exception can be
         * thrown if some change does not pass validation. None of the changes
         * is applied in such case.
         * The transaction is cleared after return.
         */
        void
        Commit();

        /** Cancel all accumulated operations. New operations can be accumulated
         * and committed after that.
         */
        void
        Cancel();

        /** Add new category.
         *
         * @param path Path of the new category. Last component is the new
         *      category name. All the preceding components should name existing
         *      categories. Special category path ":" corresponds to root
         *      category and is a way to specify its options.
         * @param options Category creation options.
         * @return New category handle.
         * @throws InvalidOpException if the change conflicts with already
         *      queued operations.
         */
        Category
        AddCategory(const Path &path,
                    const Category::Options &options = Category::Options());

        /** Add new item.
         *
         * @param path Path of the new item. Last component is the new item
         *      name. All the preceding components should name existing
         *      categories.
         * @param value Value for the item. Should not be empty value.
         * @param options Item creation options.
         * @return New item handle.
         * @throws InvalidOpException if the change conflicts with already
         *      queued operations.
         */
        Item
        AddItem(const Path &path, const Value &value,
                const Item::Options &options = Item::Options());

        /** Add new item.
         *
         * @param path Path of the new item. Last component is the new item
         *      name. All the preceding components should name existing
         *      categories.
         * @param value Value for the item. Should not be empty value.
         * @param options Item creation options.
         * @return New item handle.
         * @throws InvalidOpException if the change conflicts with already
         *      queued operations.
         */
        Item
        AddItem(const Path &path, Value &&value,
                const Item::Options &options = Item::Options());

        /** Delete a node (either item or category).
         * @param path Node path to delete.
         * @throws InvalidOpException if the change conflicts with already
         *      queued operations.
         */
        void
        Delete(const Path &path);

        /** Delete all the content. */
        void
        DeleteAll();

        /** Modify item value. */
        void
        Modify(const Path &path, const Value &value);

        /** Modify item value. */
        void
        Modify(const Path &path, Value &&value);

    private:
        friend class Properties;

        /** Represents transaction log record. Each record is either one or set
         * of aggregated pending operations.
         */
        class Record {
        public:
            /** Operation type. */
            enum class Type {
                /** Modify item value. */
                MODIFY,
                /** Add new item or category. */
                ADD,
                /** Delete item or category. Empty path indicates entire clearing. */
                DELETE
            };

            /** Operation type. */
            Type type;
            /** Affected path. Empty for root. */
            Path path;
            /** New node(s) for add operation. Can be subtree. */
            Node::Ptr newNode;
            /** New node name. Referenced by Node::_name. */
            std::string nodeName;
            /** New value for item modification operation. */
            Value newValue;
        };

        Properties *_props;
        /** Transaction log with all pending operations. */
        std::list<Record> _log;

        /** Check if the path is suitable for node addition.
         *
         * @param path Path for node to add.
         * @return Pointer to parent node if such exists in the transaction,
         *      nullptr if new node should be created. Pointer to the existing
         *      addition record if suitable found, nullptr otherwise.
         * @throws InvalidOpException if the addition conflicts with already
         *      queued operations.
         */
        std::pair<CategoryNode *, Record *>
        _CheckAddition(const Path &path);

        /** Check if the path is suitable for node deletion. In case there is
         * add or modify records with the same path they are deleted from the
         * transaction if "apply" argument is true.
         *
         * @param path Path for node to delete.
         * @return true if deletion record should be created, false if it is
         *      already covered by another deletion record.
         */
        bool
        _CheckDeletion(const Path &path, bool apply);

        /** Check if path is suitable for item modification.
         *
         * @param path Path for item to modify.
         * @return Existing value storage to save new value to.
         * @throws InvalidOpException if the modification conflicts with already
         *      queued operations.
         */
        Value *
        _CheckModification(const Path &path, Value::Type newType);

        Node::Ptr
        _AddItem(const Path &path, const Item::Options &options);

        Value *
        _Modify(const Path &path, Value::Type newType);
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

    /** Serialize the properties sheet to the XML document. */
    void
    Save(Xml &xml);

    /** Open a new transaction for the sheet modification. Changes made in scope
     * of this transaction are committed when transaction Commit() method is
     * called.
     *
     * @return New transaction object.
     */
    Transaction::Ptr
    OpenTransaction();

    /** Add new category.
     *
     * @param path Path of the new category. Last component is the new
     *      category name. All the preceding components should name existing
     *      categories. Special category path ":" corresponds to root
     *      category and is a way to specify its options.
     * @param options Category creation options.
     * @return New category handle.
     * @throws InvalidOpException if the change conflicts is not legal.
     * @throws ValidationException if a validator rejected the changes.
     */
    Category
    AddCategory(const Path &path,
                const Category::Options &options = Category::Options());

    /** Add new item.
     *
     * @param path Path of the new item. Last component is the new item
     *      name. All the preceding components should name existing
     *      categories.
     * @param value Value for the item. Should not be empty value.
     * @param options Item creation options.
     * @return New item handle.
     * @throws InvalidOpException if the change conflicts is not legal.
     * @throws ValidationException if a validator rejected the changes.
     */
    Item
    AddItem(const Path &path, const Value &value,
            const Item::Options &options = Item::Options());

    /** Add new item.
     *
     * @param path Path of the new item. Last component is the new item
     *      name. All the preceding components should name existing
     *      categories.
     * @param value Value for the item. Should not be empty value.
     * @param options Item creation options.
     * @return New item handle.
     * @throws InvalidOpException if the change conflicts is not legal.
     * @throws ValidationException if a validator rejected the changes.
     */
    Item
    AddItem(const Path &path, Value &&value,
            const Item::Options &options = Item::Options());

    /** Delete a node (either item or category).
     * @param path Node path to delete.
     * @throws InvalidOpException if the change conflicts is not legal.
     * @throws ValidationException if a validator rejected the changes.
     */
    void
    Delete(const Path &path);

    /** Modify item value.
     * @throws InvalidOpException if the change conflicts is not legal.
     * @throws ValidationException if a validator rejected the changes.
     */
    void
    Modify(const Path &path, const Value &value);

    /** Modify item value.
     * @throws InvalidOpException if the change conflicts is not legal.
     * @throws ValidationException if a validator rejected the changes.
     */
    void
    Modify(const Path &path, Value &&value);

private:
    /** Root category. Its display name contains optional title for the whole
     * properties sheet. Description corresponds to the whole properties sheet
     * optional description.
     */
    Node::Ptr _root;
    /** Mutex for properties tree modification. */
    std::mutex _mutex;

    /** Load category from XML element.
     *
     * @param trans Target transaction.
     * @param catEl XML element with category description.
     * @param path Preceding path.
     * @param isRoot Is it root category.
     */
    void
    _LoadCategory(Transaction::Ptr trans, Xml::Element catEl, const Path &path,
                  bool isRoot = false);

    /** Load item from XML element.
     *
     * @param trans Target transaction.
     * @param itemEl XML element with item description.
     * @param path Preceding path.
     */
    void
    _LoadItem(Transaction::Ptr trans, Xml::Element itemEl, const Path &path);

    /** Transaction is committed. All data validated and either InvalidOpException
     * or ValidationException is thrown.
     * @param trans Transaction to commit;
     */
    void
    _CommitTransaction(Transaction &trans);

    /** Check validity of additions in the transactions. */
    void
    _CheckAdditions(Transaction &trans);

    /** Check validity of deletions in the transactions. */
    void
    _CheckDeletions(Transaction &trans);

    /** Apply additions in the transactions. */
    void
    _ApplyAdditions(Transaction &trans);

    /** Find node is exists. */
    Node::Ptr
    _LookupNode(const Path &path);
};

} /* namespace adk */

#endif /* PROPERTIES_H_ */
