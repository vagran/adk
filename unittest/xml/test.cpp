/* /ADK/unittest/signal/test.cpp
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file test.cpp
 * Unit tests for XML manipulation.
 */

#include <adk.h>
#include <adk_ut.h>

using namespace adk;

UT_TEST("Basic functionality")
{
    Xml xml;
    xml.Load(GetResource("test.xml").GetString());

    UT(xml.Root().Name().c_str()) == UT("test");

    {
        auto e = xml.Child("item");
        UT(static_cast<bool>(e)) == UT_TRUE;
        UT(e.Value().c_str()) == UT("value 1 <>\"&'");
        UT(e.Attr("attr").Value().c_str()) == UT("attr 1 value");

        e = e.NextSibling("item");
        UT(static_cast<bool>(e)) == UT_TRUE;
        UT(e.Value().c_str()) == UT("value 2");
        UT(e.Attr("attr").Value().c_str()) == UT("attr 2 value");

        e = e.NextSibling("item");
        UT(static_cast<bool>(e)) == UT_TRUE;
        UT(e.Value().c_str()) == UT("value 3");
        UT(e.Attr("attr").Value().c_str()) == UT("attr 3 value");

        e = e.NextSibling();
        UT(static_cast<bool>(e)) == UT_TRUE;
        UT(e.Name().c_str()) == UT("parent");

        e = e.NextSibling();
        UT(static_cast<bool>(e)) == UT_FALSE;
    }

    {
        auto e = xml.Child();
        UT(static_cast<bool>(e)) == UT_TRUE;
        UT(e.Value().c_str()) == UT("value 1 <>\"&'");
        UT(e.Attr("attr").Value().c_str()) == UT("attr 1 value");

        e = e.NextSibling();
        UT(static_cast<bool>(e)) == UT_TRUE;
        UT(e.Value().c_str()) == UT("value 2");
        UT(e.Attr("attr").Value().c_str()) == UT("attr 2 value");

        e = e.NextSibling();
        UT(static_cast<bool>(e)) == UT_TRUE;
        UT(e.Value().c_str()) == UT("value 3");
        UT(e.Attr("attr").Value().c_str()) == UT("attr 3 value");

        e = e.NextSibling();
        UT(static_cast<bool>(e)) == UT_TRUE;
        UT(e.Name().c_str()) == UT("parent");

        e = e.NextSibling();
        UT(static_cast<bool>(e)) == UT_FALSE;
    }

    {
        auto e = xml.Child("parent");
        UT(static_cast<bool>(e)) == UT_TRUE;
        e = e.Child("child");
        UT(static_cast<bool>(e)) == UT_TRUE;
        UT(e.Value().c_str()) == UT("value 1");
    }

    {
        auto e = xml.Child();
        UT(e.Value().c_str()) == UT("value 1 <>\"&'");
    }

    std::string s;
    xml.Save(s);
    const char *expected =
"<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n\
<test>\n\
  <item attr=\"attr 1 value\" attr2=\"test attr &lt;&gt;&quot;&amp;&apos;\">value 1 &lt;&gt;&quot;&amp;&apos;</item>\n\
  <item attr=\"attr 2 value\">value 2</item>\n\
  <item attr=\"attr 3 value\">value 3</item>\n\
  <parent>\n\
    <child>value 1</child>\n\
    <child attr=\"attr value\">value</child>\n\
  </parent>\n\
</test>\n\
";
    UT(s.c_str()) == UT(expected);
}

void
CheckElementSequence(const Xml::Element::Iterable &seq,
                     const std::list<std::string> &expected)
{
    auto expected_it = expected.begin();
    for (Xml::Element e: seq) {
        UT(e.Name().c_str()) == UT(expected_it->c_str());
        expected_it++;
    }
    UT(expected_it == expected.end()) == UT_TRUE;
}

void
CheckAttributesSequence(const Xml::Attribute::Iterable &seq,
                        const std::list<std::string> &expected)
{
    auto expected_it = expected.begin();
    for (Xml::Attribute attr: seq) {
        std::string value = attr.Name() + '=' + attr.Value();
        UT(value.c_str()) == UT(expected_it->c_str());
        expected_it++;
    }
    UT(expected_it == expected.end()) == UT_TRUE;
}

UT_TEST("Iteration")
{
    Xml xml;
    xml.Load(GetResource("test.xml").GetString());

    CheckElementSequence(xml.Children("item"),
                         std::list<std::string> {"item", "item", "item"});
    CheckElementSequence(xml.Children(),
                         std::list<std::string> {"item", "item", "item", "parent"});

    auto e = xml.Child("item");
    CheckAttributesSequence(e.Attributes(),
                            std::list<std::string> {"attr=attr 1 value", "attr2=test attr <>\"&'"});
}
