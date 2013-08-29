/* /ADK/include/adk/xml.h
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file xml.h
 * XML manipulation functionality.
 */

#ifndef XML_H_
#define XML_H_

#include <expat.h>

namespace adk {

/** Represents XML document manipulator. */
class Xml {
private:
    /** Numerical name ID. */
    typedef u32 NameId;

    class ElementNode;

    /** Represents attribute node in the document. */
    class AttributeNode {
    public:
        typedef std::unique_ptr<AttributeNode> Ptr;

        AttributeNode(ElementNode &e, NameId nameId = 0,
                      const std::string &value = std::string());

        operator bool() const
        {
            return _nameId;
        }

        /** Get attribute value. */
        std::string
        Value() const
        {
            return _value;
        }

        /** Get attribute name. */
        std::string
        Name() const;

        void
        SetValue(const std::string &value);

        ElementNode *
        Element() const
        {
            return &_element;
        }

        /** Get next attribute when iterating. */
        AttributeNode *
        Next() const;

        void
        Remove();
    private:
        /** Related element. */
        ElementNode &_element;
        /** Name ID. */
        NameId _nameId;
        /** Value string. */
        std::string _value;
    };

    /** Represents element node in the document. */
    class ElementNode {
    public:
        typedef std::unique_ptr<ElementNode> Ptr;

        ElementNode(Xml &doc, NameId nameId = 0):
            _doc(doc), _nameId(nameId)
        {}

        /** Get element content. */
        std::string
        Value() const
        {
            return _value;
        }

        /** Get element name. */
        std::string
        Name() const
        {
            return _doc._GetName(_nameId);
        }

        bool
        ValueEmpty() const
        {
            return _value.empty();
        }

        void
        SetValue(const std::string &value)
        {
            _value = value;
        }

        /** Get value of the specified attribute.
         *
         * @param name Attribute name.
         * @return Attribute, nullptr if no such attribute found.
         */
        AttributeNode *
        Attribute(const std::string &name) const;

        /** Either modify existing or add new attribute. */
        AttributeNode *
        SetAttribute(const std::string &name, const std::string &value);

        /** Get first attribute for iteration. */
        AttributeNode *
        FirstAttribute() const;

        ElementNode *
        Parent() const
        {
            return _parent;
        }

        /** Get first child element with the specified name. Keep in mind that
         * for efficiency reasons items are not stored in the same order they
         * appears in the document but are alphabetically sorted.
         */
        ElementNode *
        Child(const std::string &name = std::string()) const;

        /** Get next sibling node with the specified name. */
        ElementNode *
        NextSibling(const std::string &name = std::string()) const;

        void
        Remove();

        ElementNode *
        AddChild(const std::string &name);

    private:
        friend class Xml;
        friend class AttributeNode;

        /** Reference to document. */
        Xml &_doc;
        /** Element name ID. */
        NameId _nameId;
        /** Element content. */
        std::string _value;
        /** Parent element, nullptr for root. */
        ElementNode *_parent = nullptr;
        /** List of sibling children with the same name. */
        class SiblingList {
        public:
            std::list<Ptr> list;
        };
        /** Child elements. */
        std::map<NameId, SiblingList> _children;
        /** Attributes. */
        std::map<NameId, AttributeNode::Ptr> _attrs;

        AttributeNode *
        _SetAttribute(NameId nameId, const std::string &value);

        void
        _AddCharData(const std::string &data);

        void
        _AddChild(ElementNode::Ptr &&e);
    };
public:
    /** Base class for all XML exceptions. */
    ADK_DEFINE_EXCEPTION(Exception);
    /** Parsing error exception. */
    ADK_DEFINE_DERIVED_EXCEPTION(ParseException, Exception);

    /** Attribute node handle. */
    class Attribute {
    public:
        class Iterator {
        public:
            Iterator(ElementNode *element, bool end = false):
                _element(element)
            {
                if (end) {
                    _attr = nullptr;
                } else {
                    _attr = _element->FirstAttribute();
                }
            }

            bool
            operator ==(const Iterator &it) const
            {
                return _element == it._element && _attr == it._attr;
            }

            bool
            operator !=(const Iterator &it) const
            {
                return _element != it._element || _attr != it._attr;
            }

            void
            operator ++()
            {
                _attr = _attr->Next();
            }

            void
            operator ++(int)
            {
                _attr = _attr->Next();
            }

            Attribute
            operator *() const
            {
                return _attr;
            }
        private:
            ElementNode *_element;
            AttributeNode *_attr;
        };

        class Iterable {
        public:
            Iterable(ElementNode *element):
                _element(element)
            {}

            Iterator
            begin() const
            {
                return Iterator(_element);
            }

            Iterator
            end() const
            {
                return Iterator(_element, true);
            }

            /** Check if there is something to iterate. */
            operator bool()
            {
                return begin() != end();
            }
        private:
            ElementNode *_element;
        };

        Attribute(AttributeNode *node = nullptr):
            _node(node)
        {}

        operator bool() const
        {
            return _node != nullptr;
        }

        std::string
        Value() const
        {
            ASSERT(_node);
            return _node->Value();
        }

        std::string
        Name() const
        {
            ASSERT(_node);
            return _node->Name();
        }

        void
        SetValue(const std::string &value)
        {
            ASSERT(_node);
            _node->SetValue(value);
        }

        void
        Remove()
        {
            _node->Remove();
            _node = nullptr;
        }
    private:
        AttributeNode *_node;
    };

    /** Element node handle. */
    class Element {
    public:
        class Iterator {
        public:
            Iterator(ElementNode *node, const std::string &name = std::string()):
                _node(node), _name(name)
            {}

            bool
            operator ==(const Iterator &it) const
            {
                return _node == it._node;
            }

            bool
            operator !=(const Iterator &it) const
            {
                return _node != it._node;
            }

            void
            operator ++()
            {
                _node = _node->NextSibling(_name);
            }

            void
            operator ++(int)
            {
                _node = _node->NextSibling(_name);
            }

            Element
            operator *() const
            {
                return Element(_node);
            }
        private:
            ElementNode *_node;
            std::string _name;
        };

        class Iterable {
        public:
            Iterable(ElementNode *node, const std::string &name = std::string()):
                _node(node), _name(name)
            {}

            Iterator
            begin() const
            {
                return Iterator(_node->Child(_name), _name);
            }

            Iterator
            end() const
            {
                return Iterator(nullptr);
            }

            /** Check if there is something to iterate. */
            operator bool()
            {
                return begin() != end();
            }
        private:
            ElementNode *_node;
            std::string _name;
        };


        Element(ElementNode *node = nullptr):
            _node(node)
        {}

        operator bool() const
        {
            return _node != nullptr;
        }

        std::string
        Value() const
        {
            ASSERT(_node);
            return _node->Value();
        }

        std::string
        Name() const
        {
            ASSERT(_node);
            return _node->Name();
        }

        void
        SetValue(const std::string &value)
        {
            ASSERT(_node);
            _node->SetValue(value);
        }

        /** Check if the element value is empty. */
        bool
        ValueEmpty() const
        {
            ASSERT(_node);
            return _node->ValueEmpty();
        }

        Element
        Parent() const
        {
            ASSERT(_node);
            return _node->Parent();
        }

        Element
        Child(const std::string &name = std::string()) const
        {
            ASSERT(_node);
            return _node->Child(name);
        }

        Element
        NextSibling(const std::string &name = std::string()) const
        {
            ASSERT(_node);
            return _node->NextSibling(name);
        }

        Attribute
        Attr(const std::string &name) const
        {
            ASSERT(_node);
            return _node->Attribute(name);
        }

        Attribute
        SetAttribute(const std::string &name, const std::string &value)
        {
            ASSERT(_node);
            return _node->SetAttribute(name, value);
        }

        /** Iterate over child elements, either all or ones with the
         * specified name.
         */
        Iterable
        Children(const std::string &name = std::string()) const
        {
            return Iterable(_node, name);
        }

        /** Iterate over attributes. */
        Attribute::Iterable
        Attributes() const
        {
            return Attribute::Iterable(_node);
        }

        void
        Remove()
        {
            _node->Remove();
            _node = nullptr;
        }

        Element
        AddChild(const std::string &name)
        {
            return _node->AddChild(name);
        }
    private:
        ElementNode *_node;
    };

    Xml();

    ~Xml();

    /** Load document from the buffer. */
    void
    Load(const char *buf, size_t size = std::string::npos);

    /** Load document from the string. */
    void
    Load(const std::string &buf);

    /** Load document from the stream. */
    void
    Load(std::istream &stream);

    /** Save document to the provided stream. */
    void
    Save(std::ostream &stream);

    /** Save document into the provided string. */
    void
    Save(std::string &str);

    /** Clear all the content. */
    void
    Clear();

    /** Get root element. */
    Element
    Root() const
    {
        return _root.get();
    }

    /** Get first child of root element with the specified name. Empty element
     * if not found.
     */
    Element
    Child(const std::string &name = std::string()) const
    {
        return Root().Child(name);
    }

    /** Get root element attribute with the specified name. */
    Attribute
    Attr(const std::string &name) const
    {
        return Root().Attr(name);
    }

    /** Modify or create a new root element attribute with the specified name
     * and value.
     */
    Attribute
    SetAttribute(const std::string &name, const std::string &value)
    {
        return Root().SetAttribute(name, value);
    }

    /** Get set of root element children elements - either or with the specified name. */
    Element::Iterable
    Children(const std::string &name = std::string()) const
    {
        return Root().Children(name);
    }

    /** Get set of root element attributes. */
    Attribute::Iterable
    Attributes() const
    {
        return Root().Attributes();
    }

    /** Escape all XML entities in the string.
     * @param trimWhitespaces Trim whitespace characters at the beginning and
     *      end of the string if true.
     */
    static std::string
    EscapeEntities(const std::string &s, bool trimWhitespaces = false);

private:
    XML_Parser _parser = nullptr;
    /** Next free name ID. */
    NameId _curNameId = 1;
    /** All defined elements and attribute names stored here. */
    std::map<const std::string, NameId> _names;
    /** Index for mapping numerical IDs to symbolic names. */
    std::map<NameId, const std::string &> _namesIndex;
    /** Root element. */
    ElementNode::Ptr _root = nullptr;
    /** Current element during parsing. */
    ElementNode *_curElement = nullptr;

    /** Either find existing or add a new name to the names index.
     * @return Numerical identifier for the name.
     */
    NameId
    _AddName(const std::string &name);

    /** Get symbolic name by its ID. */
    std::string
    _GetName(NameId id) const;

    /** Get ID for the specified name. Returns zero if no such name. */
    NameId
    _GetNameId(const std::string name) const;

    void
    _StartElementHandler(const XML_Char *name, const XML_Char **attrs);

    void
    _EndElementHandler(const XML_Char *name);

    void
    _CharDataHandler(const XML_Char *s, int len);

    /** Create and setup new parser instance. */
    void
    _CreateParser();

    void
    _CreateParseException();

    /** Create a new element with the specified attributes. */
    ElementNode::Ptr
    _CreateElement(NameId nameId, const XML_Char **attrs);

    void
    _SaveElement(Element e, int indentation, std::ostream &stream);

    void
    _SaveIndentation(int indentation, std::ostream &stream);
};

} /* namespace adk */

#endif /* XML_H_ */
