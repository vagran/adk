/* /ADK/unittest/python/test.cpp
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file test.cpp
 * Unit tests for embedded Python.
 */

#include <adk.h>
#include <adk_ut.h>

using namespace adk;

UT_TEST("Basic functionality")
{
    Xml xml;
    xml.Load(GetResource("test.xml").GetString());

    UT(xml.Root().Name().c_str()) == UT_CSTR("test");

    {
        auto e = xml.Child("item");
        UT(static_cast<bool>(e)) == UT_TRUE;
        UT(e.Value().c_str()) == UT_CSTR("value 1");
        UT(e.Attr("attr").Value().c_str()) == UT_CSTR("attr 1 value");

        e = e.NextSibling("item");
        UT(static_cast<bool>(e)) == UT_TRUE;
        UT(e.Value().c_str()) == UT_CSTR("value 2");
        UT(e.Attr("attr").Value().c_str()) == UT_CSTR("attr 2 value");

        e = e.NextSibling("item");
        UT(static_cast<bool>(e)) == UT_TRUE;
        UT(e.Value().c_str()) == UT_CSTR("value 3");
        UT(e.Attr("attr").Value().c_str()) == UT_CSTR("attr 3 value");

        e = e.NextSibling();
        UT(static_cast<bool>(e)) == UT_TRUE;
        UT(e.Name().c_str()) == UT_CSTR("parent");

        e = e.NextSibling();
        UT(static_cast<bool>(e)) == UT_FALSE;
    }

    {
        auto e = xml.Child();
        UT(static_cast<bool>(e)) == UT_TRUE;
        UT(e.Value().c_str()) == UT_CSTR("value 1");
        UT(e.Attr("attr").Value().c_str()) == UT_CSTR("attr 1 value");

        e = e.NextSibling();
        UT(static_cast<bool>(e)) == UT_TRUE;
        UT(e.Value().c_str()) == UT_CSTR("value 2");
        UT(e.Attr("attr").Value().c_str()) == UT_CSTR("attr 2 value");

        e = e.NextSibling();
        UT(static_cast<bool>(e)) == UT_TRUE;
        UT(e.Value().c_str()) == UT_CSTR("value 3");
        UT(e.Attr("attr").Value().c_str()) == UT_CSTR("attr 3 value");

        e = e.NextSibling();
        UT(static_cast<bool>(e)) == UT_TRUE;
        UT(e.Name().c_str()) == UT_CSTR("parent");

        e = e.NextSibling();
        UT(static_cast<bool>(e)) == UT_FALSE;
    }

    {
        auto e = xml.Child("parent");
        UT(static_cast<bool>(e)) == UT_TRUE;
        e = e.Child("child");
        UT(static_cast<bool>(e)) == UT_TRUE;
        UT(e.Value().c_str()) == UT_CSTR("value 1");
    }

    {
        auto e = xml.Child();
        UT(e.Value().c_str()) == UT_CSTR("value 1");
    }
}
