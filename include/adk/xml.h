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
public:
    /** Base class for all XML exceptions. */
    ADK_DEFINE_EXCEPTION(Exception);
    /** Parsing error exception. */
    ADK_DEFINE_DERIVED_EXCEPTION(ParseException, Exception);
    /** Element or attribute with the specified name not found. */
    ADK_DEFINE_DERIVED_EXCEPTION(NotFoundException, Exception);

    class Element {
    public:
        typedef std::unique_ptr<Element> Ptr;

        std::string
        Value() const
        {
            return _value;
        }

        std::string
        Name() const
        {
            return _doc._GetName(_nameId);
        }

        /** Get value of the specified attribute.
         *
         * @param name Attribute name.
         * @return Attribute value.
         * @throws NotFoundException If attribute with the specified name is not
         *      found.
         */
        std::string
        Attribute(const std::string &name) const;

        /** Check if the element has attribute with the specified name. */
        bool
        HasAttribute(const std::string &name) const;

        Element &
        Parent() const
        {
            if (!_parent) {
                ADK_EXCEPTION(Exception, "Parent element requested for root");
            }
            return *_parent;
        }

        bool
        IsRoot() const
        {
            return _parent == nullptr;
        }

    private:
        friend class Xml;

        /** Reference to document. */
        Xml &_doc;
        /** Element name ID. */
        NameId _nameId;
        /** Element content. */
        std::string _value;
        /** Attributes map. */
        std::map<NameId, std::string> _attrs;
        /** Parent element, nullptr for root. */
        Element *_parent = nullptr;
        /** List of sibling children with the same name. */
        class SiblingList {
        public:
            std::list<Ptr> list;
        };
        /** Child elements. */
        std::map<NameId, SiblingList> _children;

        Element(Xml &doc, NameId nameId):
            _doc(doc), _nameId(nameId)
        {}

        void
        _SetAttribute(NameId nameId, const std::string &value);

        void
        _AddCharData(const std::string &data);

        void
        _AddChild(Element::Ptr &&e);
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
private:
    XML_Parser _parser = nullptr;
    /** Next free name ID. */
    NameId _curNameId = 1;
    /** All defined elements and attribute names stored here. */
    std::map<const std::string, NameId> _names;
    /** Index for mapping numerical IDs to symbolic names. */
    std::map<NameId, const std::string &> _namesIndex;
    /** Root element. */
    Element::Ptr _root = nullptr;
    /** Current element during parsing. */
    Element *_curElement = nullptr;

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
    Element::Ptr
    _CreateElement(NameId nameId, const XML_Char **attrs);
};

} /* namespace adk */

#endif /* XML_H_ */
