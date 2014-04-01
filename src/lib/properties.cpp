/* /ADK/src/lib/properties.cpp
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
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

Properties::Value::Type
Properties::Value::TypeFromString(const std::string &typeStr)
{
    if (typeStr == "integer") {
        return Type::INTEGER;
    } else if (typeStr == "float") {
        return Type::FLOAT;
    } else if (typeStr == "boolean") {
        return Type::BOOLEAN;
    } else if (typeStr == "string") {
        return Type::STRING;
    }
    return Type::NONE;
}

Properties::Value
Properties::Value::FromString(Type type, const std::string &s)
{
    size_t pos;

    switch (type) {

    case Type::INTEGER:
        long l;
        try {
            l = std::stol(s, &pos, 0);
        } catch (std::invalid_argument &) {
            ADK_EXCEPTION(ParseException,
                          "Cannot convert string to integer: " << s);
        } catch (std::out_of_range &) {
            ADK_EXCEPTION(ParseException,
                          "Value out of range: " << s);
        }
        if (pos != s.size()) {
            ADK_EXCEPTION(ParseException,
                          "Trailing garbage in integer value: " << s);
        }
        return Value(l);

    case Type::FLOAT:
        double d;
        try {
            d = std::stod(s, &pos);
        } catch (std::invalid_argument &) {
            ADK_EXCEPTION(ParseException,
                          "Cannot convert string to float: " << s);
        } catch (std::out_of_range &) {
            ADK_EXCEPTION(ParseException,
                          "Value out of range: " << s);
        }
        if (pos != s.size()) {
            ADK_EXCEPTION(ParseException,
                          "Trailing garbage in float value: " << s);
        }
        return Value(d);

    case Type::BOOLEAN:
        bool b;
        if (s == "true") {
            b = true;
        } else if (s == "false") {
            b = false;
        } else {
            ADK_EXCEPTION(ParseException, "Invalid boolean value: " << s);
        }
        return Value(b);

    case Type::STRING:
        return Value(s);

    case Type::NONE:
        break;
    }
    ENSURE(false);
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

std::string
Properties::Path::First() const &
{
    ASSERT(_components.size());
    return _components.front();
}

std::string
Properties::Path::First() &&
{
    ASSERT(_components.size());
    return std::move(_components).front();
}

std::string
Properties::Path::Last() const &
{
    ASSERT(_components.size());
    return _components.back();
}

std::string
Properties::Path::Last() &&
{
    ASSERT(_components.size());
    return std::move(_components).back();
}

bool
Properties::Path::operator ==(const Path &path) const
{
    if (_components.size() != path._components.size()) {
        return false;
    }
    for (size_t i = 0; i < _components.size(); i++) {
        if (_components[i] != path._components[i]) {
            return false;
        }
    }
    return true;
}

bool
Properties::Path::operator !=(const Path &path) const
{
    return !(*this == path);
}

Properties::Path
Properties::Path::Parent() const &
{
    size_t size = _components.size();
    if (size == 0) {
        return Path();
    }
    return SubPath(0, size - 1);
}

Properties::Path
Properties::Path::Parent() &&
{
    size_t size = _components.size();
    if (size == 0) {
        return *this;
    }
    return std::move(*this).SubPath(0, size - 1);
}

/* ****************************************************************************/
/* Properties::Node class. */

Properties::Node::Node(bool isItem):
    _isItem(isItem)
{}

Properties::Node::~Node()
{}

Properties::Node::Ptr
Properties::Node::GetPtr()
{
    return shared_from_this();
}

bool
Properties::Node::IsItem() const
{
    return _isItem;
}

bool
Properties::Node::IsCategory() const
{
    return !_isItem;
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

std::string
Properties::Node::Name() const
{
    return *_name;
}

void
Properties::Node::Unlink()
{
    if (!_parent) {
        return;
    }
    _parent->Category().UnlinkChild(*_name);
    _parent = nullptr;
    _name = nullptr;
}

/* Properties::ItemNode class. */

Properties::ItemNode::Ptr
Properties::ItemNode::Create()
{
    return std::make_shared<ItemNode>();
}

Properties::ItemNode::ItemNode():
    Node(true)
{}

/* Properties::CategoryNode class. */

Properties::CategoryNode::Ptr
Properties::CategoryNode::Create()
{
    return std::make_shared<CategoryNode>();
}

Properties::CategoryNode::CategoryNode():
    Node(false)
{}

Properties::Node::Ptr
Properties::CategoryNode::Find(const Path &path, bool itemInPathFatal)
{
    CategoryNode *node = this;
    for (size_t idx = 0; idx < path.Size(); idx++) {
        auto it = node->_children.find(path[idx]);
        if (it == node->_children.end()) {
            return nullptr;
        }
        if (it->second->IsItem()) {
            if (idx < path.Size() - 1) {
                if (itemInPathFatal) {
                    ADK_EXCEPTION(InvalidOpException,
                                  "Item found during path lookup");
                }
                return nullptr;
            }
            return it->second;
        }
        node = &it->second->Category();
    }
    return node->GetPtr();
}

void
Properties::CategoryNode::AddChild(const std::string &name, Ptr node)
{
    ASSERT(_children.find(name) == _children.end());
    auto res = _children.emplace(name, node);
    node->_name = &res.first->first;
    node->_parent = this;
}

void
Properties::CategoryNode::UnlinkChild(const std::string &name)
{
    auto it = _children.find(name);
    ASSERT(it != _children.end());
    it->second->_name = nullptr;
    it->second->_parent = nullptr;
    _children.erase(it);
}

/* ****************************************************************************/
/* Properties::Category class. */

Properties::Category::Category(CategoryNode *node):
    _node(node)
{}

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

Properties::Item::Item(ItemNode *node):
    _node(node)
{}

Properties::Value::Type
Properties::Item::Type() const
{
    ASSERT(_node);
    return _node->value.GetType();
}

Properties::Value
Properties::Item::Val() const
{
    ASSERT(_node);
    return _node->value;
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

Properties::Transaction::Ptr
Properties::Transaction::Create(Properties *props)
{
    return std::make_shared<Transaction>(props);
}

Properties::Transaction::Transaction(Properties *props):
    _props(props)
{}

Properties::Transaction::Transaction(Transaction &&trans):
    _props(trans._props), _log(std::move(trans._log))
{
    trans._props = nullptr;
}

Properties::Transaction::~Transaction()
{}

void
Properties::Transaction::Commit()
{
    _props->_CommitTransaction(*this);
    _log.clear();
}

void
Properties::Transaction::Cancel()
{
    _log.clear();
}

Properties::Category
Properties::Transaction::AddCategory(const Path &path,
                                     const Category::Options &options __UNUSED)
{
    auto res = _CheckAddition(path);
    CategoryNode *cn = res.first;
    Node::Ptr node = CategoryNode::Create();
    if (cn) {
        cn->AddChild(path.Last(), node);
        //XXX
        return &node->Category();
    }
    _log.emplace_back();
    Record &rec = _log.back();
    rec.type = Record::Type::ADD;
    if (path.Size() != 0) {
        rec.nodeName = path.Last();
    }
    rec.newNode = node;
    rec.path = path;
    node->_name = &rec.nodeName;
    //XXX
    return &node->Category();
}

Properties::Node::Ptr
Properties::Transaction::_AddItem(const Path &path, const Item::Options &options __UNUSED)
{
    if (path.Size() == 0) {
        ADK_EXCEPTION(InvalidOpException, "Cannot add item - root should be category");
    }
    auto res = _CheckAddition(path);
    CategoryNode *cn = res.first;
    Node::Ptr node = ItemNode::Create();
    if (cn) {
        //XXX
        auto res = cn->_children.emplace(path.Last(), node);
        node->_name = &res.first->first;
        node->_parent = cn;
        //XXX
        return node;
    }
    _log.emplace_back();
    Record &rec = _log.back();
    rec.type = Record::Type::ADD;
    rec.nodeName = path.Last();
    rec.newNode = node;
    rec.path = path;
    node->_name = &rec.nodeName;
    //XXX
    return node;
}

Properties::Item
Properties::Transaction::AddItem(const Path &path, const Value &value,
                                 const Item::Options &options)
{
    Node::Ptr node = _AddItem(path, options);
    node->Item().value = value;
    return &node->Item();
}

Properties::Item
Properties::Transaction::AddItem(const Path &path, Value &&value,
                                 const Item::Options &options)
{
    Node::Ptr node = _AddItem(path, options);
    node->Item().value = std::move(value);
    return &node->Item();
}

void
Properties::Transaction::Delete(const Path &path)
{
    bool needRec = _CheckDeletion(path, false);
    _CheckDeletion(path, true);
    if (needRec) {
        _log.emplace_back();
        Record &rec = _log.back();
        rec.type = Record::Type::DELETE;
        rec.path = path;
    }
}

void
Properties::Transaction::DeleteAll()
{
    _log.clear();
    _log.emplace_back();
    Record &rec = _log.back();
    rec.type = Record::Type::DELETE;
}

void
Properties::Transaction::Modify(const Path &path, const Value &value)
{
    Node::Ptr node = _Modify(path, value.GetType());
    //XXX
    if (node->IsItem()) {
        node->Item().value = value;
    }
}

void
Properties::Transaction::Modify(const Path &path, Value &&value)
{
    Node::Ptr node = _Modify(path, value.GetType());
    if (node->IsItem()) {
        node->Item().value = std::move(value);
    }
}

Properties::Node::Ptr
Properties::Transaction::_Modify(const Path &path, Value::Type newType)
{
    Node::Ptr node = _CheckModification(path, newType);
    if (node) {
        return node;
    }
    node = ItemNode::Create();
    _log.emplace_back();
    Record &rec = _log.back();
    rec.type = Record::Type::MODIFY;
    rec.path = path;
    if (path.Size()) {
        rec.nodeName = path.Last();
    }
    rec.newNode = node;
    rec.path = path;
    node->_name = &rec.nodeName;
    return node;
}

Properties::Node::Ptr
Properties::Transaction::_CheckModification(const Path &path, Value::Type newType)
{
    for (auto it = _log.begin(); it != _log.end();) {
        Record &rec = *it;
        size_t len = path.HasCommonPrefix(rec.path);

        if (rec.type == Record::Type::ADD) {
            if (len == rec.path.Size()) {
                Node::Ptr node;
                if (len == path.Size()) {
                    node = rec.newNode;
                } else {
                    if (rec.newNode->IsItem()) {
                        ADK_EXCEPTION(InvalidOpException,
                                      "Cannot modify node - item node in preceding "
                                      "path in addition record");
                    }
                    Path parentSubpath = path.SubPath(rec.path.Size(),
                                                      path.Size() - rec.path.Size());
                    node = rec.newNode->Category().Find(parentSubpath, true);
                }
                if (node) {
                    if (node->IsItem() && node->Item().value.GetType() != newType) {
                        ADK_EXCEPTION(InvalidOpException,
                                      "Cannot modify node - the value type does not "
                                      "match previously specified value type in "
                                      "found addition record");
                    }
                    return node;
                } else {
                    ADK_EXCEPTION(InvalidOpException,
                                  "Cannot modify node - not found in "
                                  "existing addition record");
                }
            }

        } else if (rec.type == Record::Type::DELETE) {
            if (len == rec.path.Size()) {
                ADK_EXCEPTION(InvalidOpException,
                              "Cannot modify node - the specified path previously deleted");
            }

        } else if (rec.type == Record::Type::MODIFY) {
            if (len == path.Size() && len == rec.path.Size()) {
                if (rec.newNode->Item().value.GetType() != newType) {
                    ADK_EXCEPTION(InvalidOpException,
                                  "Cannot modify node - the value type does not "
                                  "match previously specified value type");
                }
                return rec.newNode;
            }
        }

        it++;
    }
    return nullptr;
}

bool
Properties::Transaction::_CheckDeletion(const Path &path, bool apply)
{
    bool needRec = true;
    for (auto it = _log.begin(); it != _log.end();) {
        Record &rec = *it;
        size_t len = path.HasCommonPrefix(rec.path);

        if (rec.type == Record::Type::DELETE) {
            if (len == rec.path.Size()) {
                /* Already deleted. */
                ADK_EXCEPTION(InvalidOpException,
                              "Cannot delete node - the specified path was "
                              "already deleted");
            } else if (len == path.Size()) {
                /* New record covers the found one so it can be deleted. */
                if (apply) {
                    it = _log.erase(it);
                    continue;
                }
            }
            /* else unrelated deletion, should create separate record */
        } else if (rec.type == Record::Type::MODIFY) {
            if (len == path.Size()) {
                /* Modified node is deleted so delete the record. */
                if (apply) {
                    it = _log.erase(it);
                    continue;
                }
            } else {
                ADK_EXCEPTION(InvalidOpException,
                              "Cannot delete node - have pending item "
                              "modification record with the same prefix");
            }
        } else if (rec.type == Record::Type::ADD) {
            if (len == path.Size()) {
                /* Added subtree deleted. */
                if (len == rec.path.Size()) {
                    /* Just delete the previous added node. */
                    needRec = false;
                }
                if (apply) {
                    it = _log.erase(it);
                    continue;
                }
            } else if (len == rec.path.Size()) {
                needRec = false;
                /* Deleted node should be one of the previously added. */
                if (rec.newNode->IsItem()) {
                    ADK_EXCEPTION(InvalidOpException,
                                  "Cannot delete node - added item node exists "
                                  "in the preceding path");
                }
                Node::Ptr node = rec.newNode->Category().Find(
                    path.SubPath(rec.path.Size(), path.Size() - rec.path.Size()));
                if (!node) {
                    ADK_EXCEPTION(InvalidOpException,
                                  "Cannot delete node - not found in the previously "
                                  "added subtree");
                }
                if (apply) {
                    node->Unlink();
                }
            }
            /* else unrelated node deleted, just place the record. */
        }
        it++;
    }
    return needRec;
}

std::pair<Properties::CategoryNode *, Properties::Transaction::Record *>
Properties::Transaction::_CheckAddition(const Path &path)
{
    for (Record &rec: _log) {
        size_t len = path.HasCommonPrefix(rec.path);

        if (rec.type == Record::Type::DELETE) {
            if (len == path.Size() && len < rec.path.Size()) {
                ADK_EXCEPTION(InvalidOpException,
                              "Cannot add node - same path exists in pending "
                              "delete record");
            }
        } else if (rec.type == Record::Type::MODIFY) {
            if (len == rec.path.Size()) {
                ADK_EXCEPTION(InvalidOpException,
                              "Cannot add node - have pending item modification "
                              "record with the same prefix");
            }
            if (len == path.Size() && len < rec.path.Size()) {
                ADK_EXCEPTION(InvalidOpException,
                              "Cannot add node - same path exists in pending "
                              "modification record");
            }
        } else if (rec.type == Record::Type::ADD) {
            if (len == path.Size()) {
                ADK_EXCEPTION(InvalidOpException,
                              "Cannot add node - same path exists in pending "
                              "addition record");
            }
            if (len == rec.path.Size()) {
                /* Adding child to previous added node. */
                if (rec.newNode->IsItem()) {
                    ADK_EXCEPTION(InvalidOpException,
                                  "Cannot add node - item node exists in the "
                                  "preceding path");
                }
                /* Find node to insert the new one to. */
                Path parentSubpath = path.SubPath(rec.path.Size(),
                                                  path.Size() - rec.path.Size() - 1);
                Node::Ptr node = rec.newNode->Category().Find(parentSubpath);
                if (node) {
                    if (node->IsItem()) {
                        ADK_EXCEPTION(InvalidOpException,
                                      "Cannot add node - item node exists in the "
                                      "preceding path");
                    }
                    if (node->Category().Find(path.SubPath(path.Size() - 1, 1))) {
                        ADK_EXCEPTION(InvalidOpException,
                                      "Cannot add node - same node already added");
                    }
                    return {&node->Category(), &rec};
                } else {
                    ADK_EXCEPTION(InvalidOpException,
                                  "Cannot add node - parent node not found in "
                                  "existing addition record");
                }
            }
        }
    }

    /* Check once more if not found existing added subtree. */
    for (Record &rec: _log) {
        size_t len = path.HasCommonPrefix(rec.path);
        if (rec.type == Record::Type::DELETE) {
            if (len == rec.path.Size() && len < path.Size()) {
                ADK_EXCEPTION(InvalidOpException,
                              "Cannot add node - preceding path was previously "
                              "deleted");
            }
        }
    }
    return {nullptr, nullptr};
}

/* ****************************************************************************/
/* Properties class. */

Properties::Properties()
{}

Properties::Properties(const Xml &xml)
{
    Load(xml);
}

void
Properties::Clear()
{
    Transaction::Ptr t = OpenTransaction();
    t->DeleteAll();
    t->Commit();
}

Properties::Category
Properties::AddCategory(const Path &path, const Category::Options &options)
{
    Transaction::Ptr t = OpenTransaction();
    Category res = t->AddCategory(path, options);
    t->Commit();
    return res;
}

Properties::Item
Properties::AddItem(const Path &path, const Value &value,
                    const Item::Options &options)
{
    Transaction::Ptr t = OpenTransaction();
    Item res = t->AddItem(path, value, options);
    t->Commit();
    return res;
}

Properties::Item
Properties::AddItem(const Path &path, Value &&value, const Item::Options &options)
{
    Transaction::Ptr t = OpenTransaction();
    Item res = t->AddItem(path, std::move(value), options);
    t->Commit();
    return res;
}

void
Properties::Delete(const Path &path)
{
    Transaction::Ptr t = OpenTransaction();
    t->Delete(path);
    t->Commit();
}

void
Properties::Modify(const Path &path, const Value &value)
{
    Transaction::Ptr t = OpenTransaction();
    t->Modify(path, value);
    t->Commit();
}

void
Properties::Modify(const Path &path, Value &&value)
{
    Transaction::Ptr t = OpenTransaction();
    t->Modify(path, std::move(value));
    t->Commit();
}

void
Properties::Load(const Xml &xml)
{
    Transaction::Ptr trans = OpenTransaction();
    trans->DeleteAll();
    _LoadCategory(trans, xml.Root(), Path(), true);
    trans->Commit();
}

void
Properties::_LoadCategory(Transaction::Ptr trans, Xml::Element catEl,
                          const Path &path, bool isRoot)
{
    std::string name;
    Category::Options opts;

    if (isRoot) {
        Xml::Element e = catEl.Child("title");
        if (e) {
            opts.DispName(e.Value());
        }
    } else {
        auto nameAttr = catEl.Attr("name");
        if (!nameAttr) {
            ADK_EXCEPTION(ParseException,
                          "Required 'name' attribute not found in element " <<
                          catEl.Name() << " at " << catEl.GetLocation().Str());
        }
        name = nameAttr.Value();

        Xml::Attribute a = catEl.Attr("dispName");
        if (a) {
            opts.DispName(a.Value());
        }
    }

    Xml::Element e = catEl.Child("description");
    if (e) {
        opts.Description(e.Value());
    }

    trans->AddCategory(isRoot ? Path() : path + name, opts);

    for (Xml::Element e: catEl.Children("item")) {
        _LoadItem(trans, e, isRoot ? Path() : path + name);
    }

    for (Xml::Element e: catEl.Children("category")) {
        _LoadCategory(trans, e, isRoot ? Path() : path + name);
    }
}

void
Properties::_LoadItem(Transaction::Ptr trans, Xml::Element itemEl,
                      const Path &path)
{
    Item::Options opts;

    auto nameAttr = itemEl.Attr("name");
    if (!nameAttr) {
        ADK_EXCEPTION(ParseException,
                      "Required 'name' attribute not found in element " <<
                      itemEl.Name() << " at " << itemEl.GetLocation().Str());
    }
    std::string name = nameAttr.Value();

    Xml::Attribute a = itemEl.Attr("dispName");
    if (a) {
        opts.DispName(a.Value());
    }

    a = itemEl.Attr("units");
    if (a) {
        opts.Units(a.Value());
    }

    Xml::Element e = itemEl.Child("description");
    if (e) {
        opts.Description(e.Value());
    }

    a = itemEl.Attr("type");
    if (!a) {
        ADK_EXCEPTION(ParseException,
                      "Required 'type' attribute not found in element " <<
                      itemEl.Name() << " at " << itemEl.GetLocation().Str());
    }
    Value::Type type = Value::TypeFromString(a.Value());
    if (type == Value::Type::NONE) {
        ADK_EXCEPTION(ParseException,
                      "Invalid type specified: " << a.Value() << " at " <<
                      itemEl.GetLocation().Str());
    }

    std::string valueStr;
    a = itemEl.Attr("value");
    if (a) {
        valueStr = a.Value();
    } else {
        e = itemEl.Child("value");
        if (e) {
            valueStr = e.Value();
        } else {
            valueStr = itemEl.Value();
        }
    }
    Value value = Value::FromString(type, valueStr);

    trans->AddItem(path + name, std::move(value), opts);
}

void
Properties::_CommitTransaction(Transaction &trans)
{
    std::unique_lock<std::mutex> lock(_mutex);

    /* Check operations validity. */
    _CheckDeletions(trans);
    _CheckAdditions(trans);
    _CheckModifications(trans);

    //XXX set current transaction by guard object

    /* Run validators. */
    //XXX

    /* Apply transaction data. */
    _ApplyDeletions(trans);
    _ApplyAdditions(trans);
    _ApplyModifications(trans);
}

void
Properties::Save(Xml &xml __UNUSED)
{
    //XXX
}

Properties::Transaction::Ptr
Properties::OpenTransaction()
{
    return Transaction::Create(this);
}

void
Properties::_CheckDeletions(Transaction &trans)
{
    for (Transaction::Record &rec: trans._log) {
        if (rec.type != Transaction::Record::Type::DELETE) {
            continue;
        }
        /* Deleted nodes should exist. Root node is exception. */
        if (rec.path.Size() == 0) {
            continue;
        }
        if (!_LookupNode(rec.path)) {
            ADK_EXCEPTION(InvalidOpException,
                          "Cannot delete node - does not exists");
        }
    }
}

void
Properties::_CheckModifications(Transaction &trans)
{
    /* Ensure the referenced nodes existence. Ensure the value type is not
     * changed.
     */
    for (Transaction::Record &rec: trans._log) {
        if (rec.type != Transaction::Record::Type::MODIFY) {
            continue;
        }
        Node::Ptr node = _LookupNode(rec.path);
        if (!node) {
            ADK_EXCEPTION(InvalidOpException,
                          "Cannot modify node - does not exists");
        }
        //XXX
        if (node->IsItem()) {
            //XXX
            if (!rec.newNode->IsItem()) {
                ADK_EXCEPTION(InvalidOpException,
                              "Cannot modify node - node type mismatch");
            }
            if (node->Item().value.GetType() != rec.newNode->Item().value.GetType()) {
                ADK_EXCEPTION(InvalidOpException,
                              "Cannot modify node - time value type mismatch");
            }
        }
    }
}

void
Properties::_CheckAdditions(Transaction &trans)
{
    for (Transaction::Record &rec: trans._log) {
        if (rec.type != Transaction::Record::Type::ADD) {
            continue;
        }
        /* Parent node should exist, the added one should not. */
        if (rec.path.Size() > 0) {
            auto node = _LookupNode(rec.path.Parent());
            if (!node) {
                ADK_EXCEPTION(InvalidOpException,
                              "Cannot add node - parent does not exist");
            }
            if (!node->IsCategory()) {
                ADK_EXCEPTION(InvalidOpException,
                              "Cannot add node - parent is not category");
            }
        }
        if (_LookupNode(rec.path)) {
            /* Check whether it was deleted in this transaction. */
            bool found = false;
            for (Transaction::Record &drec: trans._log) {
                if (drec.type != Transaction::Record::Type::DELETE) {
                    continue;
                }
                if (drec.path == rec.path) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                ADK_EXCEPTION(InvalidOpException,
                              "Cannot add node - already exists");
            }
        }
    }
}

void
Properties::_ApplyAdditions(Transaction &trans)
{
    for (Transaction::Record &rec: trans._log) {
        if (rec.type != Transaction::Record::Type::ADD) {
            continue;
        }
        if (rec.path.Size() == 0) {
            _root = rec.newNode;
            _root->_name = nullptr;
        } else {
            Node::Ptr parent = _LookupNode(rec.path.Parent());
            ASSERT(parent);
            parent->Category().AddChild(rec.nodeName, rec.newNode);
        }
    }
}

void
Properties::_ApplyDeletions(Transaction &trans)
{
    for (Transaction::Record &rec: trans._log) {
        if (rec.type != Transaction::Record::Type::DELETE) {
            continue;
        }
        if (rec.path.Size() == 0) {
            _root = nullptr;
        } else {
            Node::Ptr node = _LookupNode(rec.path);
            ASSERT(node);
            node->Unlink();
        }
    }
}

void
Properties::_ApplyModifications(Transaction &trans)
{
    for (Transaction::Record &rec: trans._log) {
        if (rec.type != Transaction::Record::Type::MODIFY) {
            continue;
        }
        Node::Ptr node = _LookupNode(rec.path);
        ASSERT(node);
        //XXX
        if (node->IsItem()) {
            node->Item().value = std::move(rec.newNode->Item().value);
        }
    }
}

Properties::Node::Ptr
Properties::_LookupNode(const Path &path)
{
    //XXX current transaction
    if (!_root) {
        return nullptr;
    }
    return _root->Category().Find(path);
}
