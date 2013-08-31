/* /ADK/unittest/properties/test.cpp
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
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
    Properties props(Xml().Load(GetResource("test_props.xml").GetString()));
    //XXX
}

UT_TEST("Properties::Value class")
{
    /* Constructors. */
    {
        Properties::Value v;
        UT(v.GetType() == Properties::Value::Type::NONE) == UT_TRUE;
        UT_THROWS(v.Get<long>(), InternalErrorException);
    }

    {
        Properties::Value v(1l);
        UT(v.Get<long>()) == UT(1);
        UT(v.GetType() == Properties::Value::Type::INTEGER) == UT_TRUE;
    }

    {
        Properties::Value v(1.0f);
        UT(v.Get<double>()) == UT(1.0);
        UT(v.Get<float>()) == UT(1.0);
        UT(static_cast<double>(v)) == UT(1.0);
        UT(static_cast<float>(v)) == UT(1.0);
        UT_THROWS(v.Get<long>(), InternalErrorException);
        UT_THROWS(static_cast<long>(v), InternalErrorException);
        UT(v.GetType() == Properties::Value::Type::FLOAT) == UT_TRUE;
    }

    {
        Properties::Value v(1.0);
        UT(v.Get<double>()) == UT(1.0);
        UT(v.Get<float>()) == UT(1.0);
        UT(static_cast<double>(v)) == UT(1.0);
        UT(static_cast<float>(v)) == UT(1.0);
        UT_THROWS(v.Get<long>(), InternalErrorException);
        UT_THROWS(static_cast<long>(v), InternalErrorException);
        UT(v.GetType() == Properties::Value::Type::FLOAT) == UT_TRUE;
    }

    {
        Properties::Value v(true);
        UT(v.Get<bool>()) == UT_TRUE;
        UT(static_cast<bool>(v)) == UT_TRUE;
        UT_THROWS(v.Get<long>(), InternalErrorException);
        UT(v.GetType() == Properties::Value::Type::BOOLEAN) == UT_TRUE;
    }

    {
        Properties::Value v("test");
        UT(v.Get<std::string>().c_str()) == UT("test");
        UT(v.Get<const char *>()) == UT("test");
        UT_THROWS(v.Get<long>(), InternalErrorException);
        UT(v.GetType() == Properties::Value::Type::STRING) == UT_TRUE;
    }

    {
        Properties::Value v(1u);
        UT(v.GetType() == Properties::Value::Type::INTEGER) == UT_TRUE;
    }

    {
        Properties::Value v(1);
        UT(v.GetType() == Properties::Value::Type::INTEGER) == UT_TRUE;
    }

    {
        Properties::Value v('\0');
        UT(v.GetType() == Properties::Value::Type::INTEGER) == UT_TRUE;
    }

    /*  Check value taking away. */
    {
        Properties::Value v("test");
        UT(std::move(v).Get<std::string>().c_str()) == UT("test");
        UT(v.IsNone()) == UT_TRUE;
        UT(Properties::Value("test").Get<std::string>().c_str()) == UT("test");
    }

    {
        Properties::Value v(1);
        UT(static_cast<int>(std::move(v))) == UT(1);
        UT(v.IsNone()) == UT_TRUE;
        UT(static_cast<int>(Properties::Value(1))) == UT(1);
    }

    /* Copy and move constructors. */
    {
        Properties::Value v1("test");
        Properties::Value v2(v1);
        UT(v1.Get<std::string>().c_str()) == UT("test");
        UT(v2.Get<std::string>().c_str()) == UT("test");
    }

    {
        Properties::Value v1("test");
        Properties::Value v2(std::move(v1));
        UT(v1.IsNone()) == UT_TRUE;
        UT_THROWS(v1.Get<std::string>(), InternalErrorException);
        UT(v2.Get<std::string>().c_str()) == UT("test");
    }

    /* Assignments operators. */
    {
        Properties::Value v("test");
        std::string s("test 2");
        UT(s.c_str()) == UT("test 2");
        v = s;
        UT(v.Get<std::string>().c_str()) == UT("test 2");
    }

    {
        Properties::Value v("test");
        std::string s("test 2");
        v = std::move(s);
        UT(s.c_str()) != UT("test 2");
        UT(v.Get<std::string>().c_str()) == UT("test 2");

        v = Properties::Value("test 3");
        UT(v.Get<std::string>().c_str()) == UT("test 3");

        v = Properties::Value(1);
        UT(v.Get<int>()) == UT(1);

        v = "test";
        UT(v.Get<std::string>().c_str()) == UT("test");

        v = "test 2";
        UT(v.Get<std::string>().c_str()) == UT("test 2");
    }

    {
        Properties::Value v("test");
        v = 2;
        UT(v.Get<int>()) == UT(2);
    }

    {
        Properties::Value v1("test");
        Properties::Value v2("test 2");
        v2 = std::move(v1);
        UT(v2.Get<std::string>().c_str()) == UT("test");
        UT(v1.IsNone()) == UT_TRUE;
    }
}

UT_TEST("Properties::Path class")
{
    {
        Properties::Path p("");
        UT_BOOL(p) == UT_FALSE;
        UT(p.Str().c_str()) == UT("");
    }

    {
        Properties::Path p("test");
        UT_BOOL(p) == UT_TRUE;
        UT(p.Size()) == UT_SIZE(1);
        UT(p[0].c_str()) == UT("test");
    }

    {
        Properties::Path p("test/1/2/3");
        UT_BOOL(p) == UT_TRUE;
        UT(p.Size()) == UT_SIZE(4);
        UT(p[0].c_str()) == UT("test");
        UT(p[1].c_str()) == UT("1");
        UT(p[2].c_str()) == UT("2");
        UT(p[3].c_str()) == UT("3");
        UT(p.Str().c_str()) == UT("test/1/2/3");
    }

    {
        Properties::Path p("test/1\\/\\\\2/\\3\\");
        UT_BOOL(p) == UT_TRUE;
        UT(p.Size()) == UT_SIZE(3);
        UT(p[0].c_str()) == UT("test");
        UT(p[1].c_str()) == UT("1/\\2");
        UT(p[2].c_str()) == UT("\\3\\");
        UT(p.Str().c_str()) == UT("test/1\\/\\\\2/\\\\3\\\\");
    }

    {
        Properties::Path p("test.1.2.3", '.');
        UT_BOOL(p) == UT_TRUE;
        UT(p.Size()) == UT_SIZE(4);
        UT(p[0].c_str()) == UT("test");
        UT(p[1].c_str()) == UT("1");
        UT(p[2].c_str()) == UT("2");
        UT(p[3].c_str()) == UT("3");
        UT(p.Str('.').c_str()) == UT("test.1.2.3");

        p += Properties::Path("4/5") + Properties::Path("6");
        UT(p.Str().c_str()) == UT("test/1/2/3/4/5/6");

        Properties::Path p2 = std::move(p);
        UT(p2.Str().c_str()) == UT("test/1/2/3/4/5/6");
        UT_BOOL(p) == UT_FALSE;
    }

    {
        UT((Properties::Path("1/2") +
            Properties::Path("3/4") +
            Properties::Path("5")).Str().c_str()) == UT("1/2/3/4/5");
    }

    {
        UT(Properties::Path("1/2/3/4").HasCommonPrefix("1/2/5/6")) == UT_SIZE(2);
        UT(Properties::Path("1/2/3/4").HasCommonPrefix("0/2/5/6")) == UT_SIZE(0);
        UT(Properties::Path("1/2/3/4").HasCommonPrefix("1/2")) == UT_SIZE(2);
        UT(Properties::Path("1/2/3/4").HasCommonPrefix("1")) == UT_SIZE(1);
        UT(Properties::Path("1/2/3/4").HasCommonPrefix("")) == UT_SIZE(0);
        UT(Properties::Path("1/2/3/4").HasCommonPrefix("1/2/3/4")) == UT_SIZE(4);
        UT(Properties::Path("1/2/3/4").HasCommonPrefix("1/2/3/4/5")) == UT_SIZE(4);

        UT(Properties::Path("").HasCommonPrefix("")) == UT_SIZE(0);
        UT(Properties::Path("").HasCommonPrefix("1")) == UT_SIZE(0);

        UT(Properties::Path("1/2/3/4").IsPrefixFor("1/2/5/6")) == UT_FALSE;
        UT(Properties::Path("1/2/3/4").IsPrefixFor("1/2/3/4")) == UT_TRUE;
        UT(Properties::Path("1/2/3/4").IsPrefixFor("1/2/3/4/5")) == UT_TRUE;
        UT(Properties::Path("1/2/3/4").IsPrefixFor("")) == UT_FALSE;
    }
}
