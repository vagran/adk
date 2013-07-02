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
public:
    /** Base class for all XML exceptions. */
    ADK_DEFINE_EXCEPTION(Exception);
    /** Parsing error exception. */
    ADK_DEFINE_DERIVED_EXCEPTION(ParseException, Exception);

    class Element {
    public:


    private:
        std::string _name, _value;
        std::map<std::string, std::string> _attrs;
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
    /** Numerical name ID. */
    typedef u32 NameId;
    /** Next free name ID. */
    NameId _curNameId = 1;
    /** All defined elements and attribute names stored here. */
    std::map<const std::string, NameId> _names;
    /** Index for mapping numerical IDs to symbolic names. */
    std::map<NameId, const std::string &> _namesIndex;

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
};

} /* namespace adk */

#endif /* XML_H_ */
