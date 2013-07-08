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

    private:
        AttributeNode *_node;
    };

    /** Element node handle. */
    class Element {
        public:
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
        private:
            ElementNode *_node;
    };

    Xml();

    ~Xml();

    /** Load document from the buffer. */
    void
    Load(const char *buf, size_t size);

    /** Load document from the string. */
    void
    Load(const std::string &buf)
    {
        Load(buf.c_str(), buf.size());
    }

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
        return _root->Child(name);
    }

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
};

} /* namespace adk */

#endif /* XML_H_ */
