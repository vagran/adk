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
        Path(const char *path, char separator = '/');
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

    class Node;

    /** Validator callback. */
    typedef Slot<void(Node)> NodeHandler;

    class NodeHandlerConnection;

    /** Item creation options. */
    class NodeOptions {
    public:
        Optional<std::string> dispName,
                              description,
                              units;

        class HandlerEntry {
        public:
            NodeHandler handler;
            NodeHandlerConnection *con;

            HandlerEntry(NodeHandler handler, NodeHandlerConnection *con):
                handler(handler), con(con)
            {}
        };

        std::list<HandlerEntry> validators,
                                listeners;

        NodeOptions &
        DispName(Optional<std::string> dispName);

        NodeOptions &
        Description(Optional<std::string> description);

        NodeOptions &
        Units(Optional<std::string> units);

        /** Add validator. It should throw ValidationException if the validation
         * fails. If connection pointer is provided the object is associated
         * with the handler and can be used to disconnect it later.
         */
        NodeOptions &
        Validator(const NodeHandler &validator,
                  NodeHandlerConnection *con = nullptr);

        NodeOptions &
        Validator(NodeHandler &&validator,
                  NodeHandlerConnection *con = nullptr);

        NodeOptions &
        Listener(const NodeHandler &listener,
                 NodeHandlerConnection *con = nullptr);

        NodeOptions &
        Listener(NodeHandler &&listener,
                 NodeHandlerConnection *con = nullptr);
    };

    /* ************************************************************************/

private:
    /** Lock object for properties sheet locking. */
    typedef std::unique_lock<std::mutex> Lock;

    /** Represents node in the properties tree. */
    class _Node: public std::enable_shared_from_this<_Node> {
    public:
        typedef std::shared_ptr<_Node> Ptr;

        /** Options attached in transaction nodes. */
        std::unique_ptr<NodeOptions> options;
        /** Current value. */
        Value value;
        /** Display name, empty if not specified. */
        Optional<std::string> dispName,
        /** Description text. */
                              description,
        /** Units string, empty if no units specified. */
                              units;

        static Ptr
        Create(Properties *props);

        _Node(Properties *props);

        Ptr
        GetPtr();

        std::string
        Name() const;

        /** Unlink the node from its parent. */
        void
        Unlink();

        /** Get child node by path. nullptr is returned if the node not found. */
        Ptr
        Find(const Path &path);

        /** Add new child node. */
        void
        AddChild(const std::string &name, Ptr node);

        /** Unlink child node with the specified name. */
        void
        UnlinkChild(const std::string &name);

        /** Lock related properties sheet. */
        Lock
        LockProps();

        /** Get this node path. Returns full path for a transaction node. */
        Path
        GetPath();

        /** Apply options to this node. */
        void
        ApplyOptions(NodeOptions &options);

        /** Traverse this and all children nodes recursively. The visitor
         * function can return false to stop traversal.
         * @return false if traversal was stopped by visitor.
         */
        bool
        Traverse(std::function<bool(_Node &)> visitor);

        /** Get parent node. */
        Ptr
        Parent() const;

        /** Has active transaction. */
        bool
        HasTransaction() const;

        /** Return next child for this node. This also accounts active
         * transaction if any.
         */
        Ptr
        NextChild(Ptr cur = nullptr);

    protected:
        friend class Transaction;
        friend class Properties;

        /** Related properties sheet. */
        Properties *_props;
        /** Internal name. */
        const std::string *_name = nullptr;
        /** Parent node, nullptr for root. */
        _Node *_parent = nullptr;
        /** Child nodes. */
        std::map<std::string, Ptr> _children;
        /** Indicates that the node is affected by current transaction. */
        bool _isChanged = false,
        /** Indicates that node is not committed. */
             _isTransaction = true;
        /** Attached validators. */
        Signal <void(Node)> _validators,
        /** Attached listeners. */
                            _listeners;
    };

public:
    class Node {
    public:
        class Iterator {
        public:
            Iterator() = default;

            Iterator(_Node::Ptr node);

            bool
            operator ==(const Iterator &it) const;

            bool
            operator !=(const Iterator &it) const;

            void
            operator ++();

            void
            operator ++(int);

            Node
            operator *() const;

            Node *
            operator ->() const;

        private:
            std::unique_ptr<Node> _node;

            void
            Next();
        };

        Node(_Node::Ptr node = nullptr);

        /** Get value type. */
        Value::Type
        Type() const;

        /** Get value. It returns a copy. */
        Value
        Val() const;

        /** Get value of the specified type. */
        template <typename T>
        T
        Val() const
        {
            return Val().Get<T>();
        }

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

        bool
        operator ==(const Node &node);

        bool
        operator !=(const Node &node);

        /** Check if the handle is not empty. */
        operator bool() const;

        /** Get value. */
        Value
        operator *() const;

        /** Get child node with the specified subpath. */
        Node
        operator [](const Path &path) const;

        /** Create and commit transaction for this node value modification. */
        Node
        operator =(const Value &value);

        /** Create and commit transaction for this node value modification. */
        Node
        operator =(Value &&value);

        /** Get this node path. */
        Path
        GetPath() const;

        /** Get parent node. Empty node returned for root. */
        Node
        Parent() const;

        /** Child nodes iteration (non-recursive). */
        Iterator
        begin() const;

        /** Child nodes iteration (non-recursive). */
        Iterator
        end() const;

    private:
        _Node::Ptr _node;

        bool
        _HasTransaction() const;
    };

    /** Connection object for handlers disconnecting. */
    class NodeHandlerConnection {
    public:
        /** Disconnect the associated handler. */
        void
        Disconnect();

        /** Check if currently associated with a connected handler. */
        operator bool();

        /** Get associated node. */
        Node
        GetNode();
    private:
        friend class Properties;

        void
        Set(_Node::Ptr node, SignalConnection<void(Node)> con);

        SignalConnection<void(Node)> _con;
        _Node::Ptr _node;
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

        /** Add new node without value (category).
         *
         * @param path Path of the new category. Last component is the new
         *      category name. All the preceding components should name existing
         *      categories. Empty path corresponds to root category and is a way
         *      to specify its options.
         * @param options Category creation options.
         * @return New category handle.
         * @throws InvalidOpException if the change conflicts with already
         *      queued operations.
         */
        Node
        Add(const Path &path,
            const NodeOptions &options = NodeOptions());

        /** Add new node with value (item).
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
        Node
        Add(const Path &path, const Value &value,
            const NodeOptions &options = NodeOptions());

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
        Node
        Add(const Path &path, Value &&value,
            const NodeOptions &options = NodeOptions());

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

        /** Modify node options. */
        void
        Modify(const Path &path, const NodeOptions &options);

        /** Modify item value. */
        void
        Modify(const Path &path, const Value &value,
               const NodeOptions &options = NodeOptions());

        /** Modify item value. */
        void
        Modify(const Path &path, Value &&value,
               const NodeOptions &options = NodeOptions());

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
            /** New node(s) for add operation or temporal node for modify
             * operation. Can be subtree for add operation.
             */
            _Node::Ptr newNode;
            /** New node name. Referenced by Node::_name. */
            std::string nodeName;
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
        std::pair<_Node::Ptr, Record *>
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
         * @return Existing to modify if any found in the transaction log.
         */
        _Node::Ptr
        _CheckModification(const Path &path, Value::Type newType);

        _Node::Ptr
        _Add(const Path &path, const NodeOptions &options);

        _Node::Ptr
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
      *
      * @throws InvalidOpException if the change is not legal.
      * @throws ValidationException if a validator rejected the changes.
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

    /** Add new node without value (category).
     *
     * @param path Path of the new category. Last component is the new
     *      category name. All the preceding components should name existing
     *      categories. Special category path ":" corresponds to root
     *      category and is a way to specify its options.
     * @param options Category creation options.
     * @return New category handle.
     * @throws InvalidOpException if the change is not legal.
     * @throws ValidationException if a validator rejected the changes.
     */
    Node
    Add(const Path &path, const NodeOptions &options = NodeOptions());

    /** Add new node with value (item).
     *
     * @param path Path of the new item. Last component is the new item
     *      name. All the preceding components should name existing
     *      categories.
     * @param value Value for the item. Should not be empty value.
     * @param options Item creation options.
     * @return New item handle.
     * @throws InvalidOpException if the change is not legal.
     * @throws ValidationException if a validator rejected the changes.
     */
    Node
    Add(const Path &path, const Value &value,
        const NodeOptions &options = NodeOptions());

    /** Add new node with value (item).
     *
     * @param path Path of the new item. Last component is the new item
     *      name. All the preceding components should name existing
     *      categories.
     * @param value Value for the item. Should not be empty value.
     * @param options Item creation options.
     * @return New item handle.
     * @throws InvalidOpException if the change is not legal.
     * @throws ValidationException if a validator rejected the changes.
     */
    Node
    Add(const Path &path, Value &&value,
        const NodeOptions &options = NodeOptions());

    /** Delete a node (either item or category).
     * @param path Node path to delete.
     * @throws InvalidOpException if the change conflicts is not legal.
     * @throws ValidationException if a validator rejected the changes.
     */
    void
    Delete(const Path &path);

    /** Modify node options.
     * @throws InvalidOpException if the change is not legal.
     * @throws ValidationException if a validator rejected the changes.
     */
    void
    Modify(const Path &path, const NodeOptions &options = NodeOptions());

    /** Modify node value.
     * @throws InvalidOpException if the change is not legal.
     * @throws ValidationException if a validator rejected the changes.
     */
    void
    Modify(const Path &path, const Value &value,
           const NodeOptions &options = NodeOptions());

    /** Modify node value.
     * @throws InvalidOpException if the change is not legal.
     * @throws ValidationException if a validator rejected the changes.
     */
    void
    Modify(const Path &path, Value &&value,
           const NodeOptions &options = NodeOptions());

    /** Get node by path. Empty path corresponds to the root node. Empty node
     * is returned if the node is not found.
     */
    Node
    Get(const Path &path = Path()) const;

    /** Get node by path. Empty path corresponds to the root node. Empty node
     * is returned if the node is not found.
     */
    Node
    operator [](const Path &path) const;

private:
    /** Helper class for setting and unsetting current transaction. */
    class TransactionGuard {
    public:
        TransactionGuard(Properties *props, Transaction *trans);

        ~TransactionGuard();

        void
        Release();

    private:
        Properties *_props;
        Lock _lock;
    };

    /** Root category. Its display name contains optional title for the whole
     * properties sheet. Description corresponds to the whole properties sheet
     * optional description.
     */
    _Node::Ptr _root;
    /** Mutex for properties tree modification. */
    mutable std::mutex _mutex,
    /** Mutex for current transaction access. */
                       _transMutex;
    /** Current transaction. */
    Transaction *_curTrans = nullptr;
    /** Thread ID of the transaction owner. */
    std::thread::id _transThread;

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

    /** Check validity of modifications in the transactions. */
    void
    _CheckModifications(Transaction &trans);

    /** Apply additions in the transactions. */
    void
    _ApplyAdditions(Transaction &trans);

    /** Apply deletions in the transactions. */
    void
    _ApplyDeletions(Transaction &trans);

    /** Apply modifications in the transaction. */
    void
    _ApplyModifications(Transaction &trans);

    /** Find node if exists. */
    _Node::Ptr
    _LookupNode(const Path &path, bool useTransaction = true,
                bool findModified = true) const;

    /** Obtain lock if necessary. */
    Lock
    _Lock() const;

    /** Reformat input text.
     * * Leading and trailing whitespaces are trimmed.
     * * Whitespaces sequence which has less than two new line characters is
     *   replaced by single space.
     * * Whitespaces sequence which has more than one new line characters is
     *   replaced by single new line character.
     */
    std::string
    ReformatText(const std::string &text);

    void
    _Validator_StringMaxLen(Node node, size_t maxLen);

    void
    _Validator_IntegerMinMax(Node node, Optional<long> minValue,
                             Optional<long> maxValue);

    void
    _Validator_FloatMinMax(Node node, Optional<double> minValue,
                           Optional<double> maxValue);
};

} /* namespace adk */

#endif /* PROPERTIES_H_ */
