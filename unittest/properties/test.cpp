/* /ADK/unittest/properties/test.cpp
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
    Properties props(Xml().Load(GetResource("test_props.xml").GetString()));

    UT_BOOL(props["Item1"]) == UT_TRUE;
    UT_BOOL(props["Item2"]) == UT_TRUE;
    UT_BOOL(props["Item3"]) == UT_FALSE;
    UT_BOOL(props["sample/Item3"]) == UT_TRUE;

    UT_INT((*props["Item1"]).GetType()) == UT_INT(Properties::Value::Type::INTEGER);
    UT_INT(*props["Item1"]) == UT(25);
    UT(props["Item1"].Val<int>()) == UT(25);
    UT_FLOAT(*props["Item2"]) == UT(120.5);
    UT((*props["sample/Item3"]).GetString().c_str()) == UT_CSTR("Test string");
    UT_INT((*props["sample/Item4"]).GetType()) == UT_INT(Properties::Value::Type::BOOLEAN);
    UT_INT((*props["sample/Item5"]).GetType()) == UT_INT(Properties::Value::Type::BOOLEAN);
    UT_BOOL(*props["sample/Item4"]) == UT_TRUE;
    UT_BOOL(*props["sample/Item5"]) == UT_TRUE;

    UT(props[Properties::Path()].DispName().c_str()) == UT_CSTR("Test properties");
    UT(props[Properties::Path()].Description().c_str()) == UT_CSTR("Root element description.");
    UT(props["sample"].Description().c_str()) == UT_CSTR(
        "Place long description here. It is automatically re-formatted.\n"
        "Empty line before new paragraph.");
    UT(props["Item1"].Description().c_str()) == UT_CSTR("Optional long description.");

    UT(props["Item2"].DispName().c_str()) == UT_CSTR("Item 2");
    UT(props["Item2"].Description().c_str()) == UT_CSTR("");
    UT(props["Item2"].Units().c_str()) == UT_CSTR("mA*h");
    UT(props["Item1"].Units().c_str()) == UT_CSTR("");
    UT(props["Item2"].Name().c_str()) == UT_CSTR("Item2");
    UT(props["Item2"].GetPath().Str().c_str()) == UT_CSTR("Item2");
    UT(props["sample/Item3"].GetPath().Str().c_str()) == UT_CSTR("sample/Item3");
    UT(props[""].GetPath().Str().c_str()) == UT_CSTR("");

    props.Modify("sample/Item3", "01234567890123456789");
    UT_THROWS(props.Modify("sample/Item3", "01234567890123456789_"),
              Properties::ValidationException);

    props.Modify("Item1", 20);
    UT_THROWS(props.Modify("Item1", 19), Properties::ValidationException);
    props.Modify("Item1", 30);
    UT_THROWS(props.Modify("Item1", 31), Properties::ValidationException);

    props.Modify("sample/Item6", 10.5);
    UT_THROWS(props.Modify("sample/Item6", 9.5), Properties::ValidationException);
    props.Modify("sample/Item6", 19.5);
    UT_THROWS(props.Modify("sample/Item6", 20.5), Properties::ValidationException);
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

    {
        UT(Properties::Path("1/2/3/4").SubPath(0).Str().c_str()) == UT("1/2/3/4");
        UT(Properties::Path("1/2/3/4").SubPath(1).Str().c_str()) == UT("2/3/4");
        UT(Properties::Path("1/2/3/4").SubPath(3).Str().c_str()) == UT("4");
        UT(Properties::Path("1/2/3/4").SubPath(4).Str().c_str()) == UT("");
        UT(Properties::Path("1/2/3/4").SubPath(0, 2).Str().c_str()) == UT("1/2");
        UT(Properties::Path("1/2/3/4").SubPath(0, 0).Str().c_str()) == UT("");
        UT(Properties::Path("1/2/3/4").SubPath(0, 4).Str().c_str()) == UT("1/2/3/4");
        UT(Properties::Path("1/2/3/4").SubPath(1, 2).Str().c_str()) == UT("2/3");
        UT(Properties::Path("1/2/3/4").SubPath(1, 0).Str().c_str()) == UT("");
        UT(Properties::Path("1/2/3/4").SubPath(4, 0).Str().c_str()) == UT("");
        UT(Properties::Path("1/2/3/4").SubPath(3, 1).Str().c_str()) == UT("4");
    }
}

UT_TEST("Properties::Transaction class")
{
    Properties props;
    Properties::Transaction::Ptr t = props.OpenTransaction();

    t->Add("a/b/c");
    UT_THROWS(t->Add("a/b/c"), Properties::InvalidOpException);
    t->Add("a/b/d");
    UT_THROWS(t->Add("a/b"), Properties::InvalidOpException);

    t->Add("a/b/e", 1);
    t->Add("a/b/e/f");
    UT_THROWS(t->Add("a/b/e/f", 1), Properties::InvalidOpException);

    t->Add("a/b/c/d");
    t->Add("a/b/c/e");
    t->Add("a/b/c/e/f");
    UT_THROWS(t->Add("a/b/c/e"), Properties::InvalidOpException);

    t->Delete("x/y/z");
    UT_THROWS(t->Add("x/y"), Properties::InvalidOpException);

    UT_THROWS(t->Delete("a/b/c/g/g/g"), Properties::InvalidOpException);
    t->Delete("a/b/c/e");
    UT_THROWS(t->Add("a/b/c/e/f"), Properties::InvalidOpException);
    t->Add("a/b/c/e");
    t->Add("a/b/c/e/f");

    t->Modify("w/a", 1);
    UT_THROWS(t->Add("w/a"), Properties::InvalidOpException);
    UT_THROWS(t->Add("w"), Properties::InvalidOpException);
    t->Add("w/b");
    t->Add("w/a/b");

    t->Delete("a");
    UT_THROWS(t->Delete("a"), Properties::InvalidOpException);
    UT_THROWS(t->Delete("a/b/c"), Properties::InvalidOpException);

    t->Cancel();
    t->Add("a/b/c/d", 1);
    t->Modify("a/b", 1);
    t->Modify("a/b/c/d", 1);
    UT_THROWS(t->Modify("a/b/c/d", "aaa"), Properties::InvalidOpException);
    UT_THROWS(t->Modify("a/b/c/d/e", 1), Properties::InvalidOpException);
    t->Modify("a/b/f", 1);

    t->Cancel();
    t->Add("a/b/c/d");
    t->Modify("a/b", 1);
    UT_THROWS(t->Modify("a/b/c/d", 1), Properties::InvalidOpException);
    UT_THROWS(t->Modify("a/b/c/d/e", 1), Properties::InvalidOpException);
    t->Modify("a/b/f", 1);

    t->Cancel();
    t->Delete("a/b/c/d");
    t->Modify("a/b", 1);
    UT_THROWS(t->Modify("a/b/c/d", 1), Properties::InvalidOpException);
    UT_THROWS(t->Modify("a/b/c/d/e", 1), Properties::InvalidOpException);
    t->Modify("a/b/c/e", 1);

    t->Cancel();
    t->Modify("a/b/c", 1);
    t->Modify("a/b/c/d", 1);
    t->Modify("a/b", 1);
    t->Modify("a/b/c", 2);
    UT_THROWS(t->Modify("a/b/c", 1.0), Properties::InvalidOpException);

    t->Cancel();
    t->Delete("a/b");
    UT_THROWS(t->Add("a/b/c"), Properties::InvalidOpException);
    t->Add("a/b");
    UT_THROWS(t->Add("a"), Properties::InvalidOpException);
    UT_THROWS(t->Add(""), Properties::InvalidOpException);

    t->Cancel();
    t->DeleteAll();
    UT_THROWS(t->Add("a/b/c"), Properties::InvalidOpException);
    UT_THROWS(t->Add("a"), Properties::InvalidOpException);
    t->Add("");
    t->Add("a");
}

UT_TEST("Transaction commit")
{
    Properties props;
    Properties::Transaction::Ptr t = props.OpenTransaction();

    UT_THROWS(props.Add("a"), Properties::InvalidOpException);

    UT_THROWS(props.Add("a/b/c"), Properties::InvalidOpException);

    t->Add("");
    t->Add("a");
    t->Add("a/b");
    t->Add("a/b/c");
    t->Commit();

    UT_THROWS(props.Add(""), Properties::InvalidOpException);
    UT_THROWS(props.Add("a/b"), Properties::InvalidOpException);
    UT_THROWS(props.Add("a/b/c"), Properties::InvalidOpException);
    props.Add("a/b/c/d");

    UT_THROWS(props.Delete("b"), Properties::InvalidOpException);
    props.Delete("a/b/c");
    props.Add("a/b/c");

    t->Delete("a/b/c");
    t->Add("a/b/d");
    t->Commit();

    props.Add("a/b/d/c");

    t->DeleteAll();
    t->Add("");
    t->Add("a");
    t->Add("a/b");
    t->Add("a/b/c");
    t->Add("a/b2");
    t->Add("a/b2/c");
    t->Add("a/b2/c2");
    t->Add("a/b3");
    t->Add("a/b3/c");
    t->Add("a/b3/c2");
    t->Commit();

    t->Delete("a/b2");
    t->Add("a/b2");
    t->Add("a/b2/c");
    t->Add("a/b2/c2");
    t->Commit();

    t->Add("a/b2/c3");
    t->Delete("a/b2");
    t->Commit();

    props.Clear();
    UT_THROWS(props.Modify("", 1), Properties::InvalidOpException);
    UT_THROWS(props.Modify("a", 1), Properties::InvalidOpException);
    t->Add("");
    t->Add("a");
    t->Add("a/b");
    t->Add("a/b/c", 1);
    t->Modify("a/b/c", 2);
    t->Commit();

    UT(props["a/b/c"].Val<int>()) == UT(2);
    UT_THROWS(props.Modify("a/b/c", "aaa"), Properties::InvalidOpException);
}

UT_TEST("Validators")
{
    Properties props;
    Properties::Transaction::Ptr t = props.OpenTransaction();

    Properties::NodeHandler blocker = Properties::NodeHandler::Make([](Properties::Node node) {
        ADK_PROPS_INVALID(node, "test validation");
    }, std::placeholders::_1);

    props.Add("");
    t->Modify("", Properties::NodeOptions().Validator(blocker));
    UT_THROWS(t->Commit(), Properties::ValidationException);
    UT_THROWS(props.Add("a",  Properties::NodeOptions().Validator(blocker)),
              Properties::ValidationException);

    Properties::NodeHandler gt10 = Properties::NodeHandler::Make([](Properties::Node node) {
        UT_BOOL(node) == UT_TRUE;
        UT(node.Type() == Properties::Value::Type::INTEGER) == UT_TRUE;
        if (node.Val<int>() <= 10) {
            ADK_PROPS_INVALID(node, "Should be greater than 10");
        }
    }, std::placeholders::_1);

    Properties::NodeHandler lt20 = Properties::NodeHandler::Make([](Properties::Node node) {
        UT_BOOL(node) == UT_TRUE;
        UT(node.Type() == Properties::Value::Type::INTEGER) == UT_TRUE;
        if (node.Val<int>() >= 20) {
            ADK_PROPS_INVALID(node, "Should be less than 20");
        }
    }, std::placeholders::_1);

    Properties::NodeHandler ne15 = Properties::NodeHandler::Make([](Properties::Node node) {
        UT_BOOL(node) == UT_TRUE;
        UT(node.Type() == Properties::Value::Type::INTEGER) == UT_TRUE;
        if (node.Val<int>() == 15) {
            ADK_PROPS_INVALID(node, "Should not be equal 15");
        }
    }, std::placeholders::_1);

    UT_THROWS(props.Add("x", 10, Properties::NodeOptions().Validator(gt10)),
              Properties::ValidationException);
    UT_BOOL(props["x"]) == UT_FALSE;
    props.Add("x", 20, Properties::NodeOptions().Validator(gt10));
    UT_BOOL(props["x"]) == UT_TRUE;
    UT_INT(props["x"].Val()) == UT(20);
    UT_THROWS(props.Modify("x", 10), Properties::ValidationException);
    UT_INT(props["x"].Val()) == UT(20);
    UT_THROWS(props.Modify("x", Properties::NodeOptions().Validator(lt20)),
              Properties::ValidationException);
    props.Modify("x", 19);
    props.Modify("x", Properties::NodeOptions().Validator(lt20));
    UT_THROWS(props.Modify("x", 10), Properties::ValidationException);
    UT_THROWS(props.Modify("x", 20), Properties::ValidationException);
    UT_INT(props["x"].Val()) == UT(19);

    Properties::NodeHandlerConnection con;
    props.Modify("x", Properties::NodeOptions().Validator(ne15, &con));
    UT_THROWS(props.Modify("x", 15), Properties::ValidationException);
    UT_INT(props["x"].Val()) == UT(19);
    con.Disconnect();
    props.Modify("x", 15);
    UT_INT(props["x"].Val()) == UT(15);

    t->Cancel();
    t->Add("sum");
    t->Add("sum/a", 1);
    t->Add("sum/b", 2);
    t->Add("sum/c", 4);
    t->Commit();

    auto Sum = [](Properties::Node node) {
        int sum = 0;
        for (Properties::Node summant: node) {
            if (summant.Type() != Properties::Value::Type::INTEGER) {
                ADK_PROPS_INVALID(node, "Should be integer");
            }
            sum += summant.Val<int>();
        }
        return sum;
    };

    UT(Sum(props["sum"])) == UT(7);

    Properties::NodeHandler sum15 = Properties::NodeHandler::Make([&Sum](Properties::Node node) {
        UT_BOOL(node) == UT_TRUE;
        int sum = Sum(node);
        if (sum != 15) {
            ADK_PROPS_INVALID(node, "Sum should be 15, have " << sum);
        }
    }, std::placeholders::_1);

    Properties::NodeHandler sum31 = Properties::NodeHandler::Make([&Sum](Properties::Node node) {
        UT_BOOL(node) == UT_TRUE;
        int sum = Sum(node);
        if (sum != 31) {
            ADK_PROPS_INVALID(node, "Sum should be 31, have " << sum);
        }
    }, std::placeholders::_1);

    UT_THROWS(props.Modify("sum", Properties::NodeOptions().Validator(sum15)),
              Properties::ValidationException);

    t->Modify("sum", Properties::NodeOptions().Validator(sum15, &con));
    t->Add("sum/d", 8);
    t->Commit();

    UT(Sum(props["sum"])) == UT(15);

    t->Modify("sum/a", 3);
    t->Modify("sum/b", 0);
    t->Commit();

    t->Delete("sum/a");
    t->Add("sum/a", 1);
    t->Modify("sum/b", 2);
    t->Commit();

    t->Delete("sum/a");
    UT_THROWS(t->Commit(), Properties::ValidationException);

    t->Cancel();
    t->Delete("sum/a");
    t->Add("sum/e", 2);
    UT_THROWS(t->Commit(), Properties::ValidationException);

    t->Cancel();
    t->Delete("sum/a");
    t->Add("sum/e", 1);
    t->Commit();

    con.Disconnect();
    t->Cancel();
    t->Modify("sum", Properties::NodeOptions().Validator(sum31));
    t->Modify("sum/e", 4);
    t->Add("sum/f", 3);
    UT_THROWS(t->Commit(), Properties::ValidationException);

    t->Cancel();
    t->Modify("sum", Properties::NodeOptions().Validator(sum31));
    t->Modify("sum/e", 4);
    t->Delete("sum/b");
    t->Add("sum/f", 15);
    t->Commit();

    t->Delete("sum");
}

UT_TEST("Listeners")
{
    Properties props;
    Properties::Transaction::Ptr t = props.OpenTransaction();

    int sum = 0;

    Properties::NodeHandler listener = Properties::NodeHandler::Make([&sum](Properties::Node node) {
        UT_BOOL(node) == UT_TRUE;
        UT(node.Type() == Properties::Value::Type::INTEGER) == UT_TRUE;
        sum += node.Val<int>();
    }, std::placeholders::_1);

    props.Add("", 0x1, Properties::NodeOptions().Listener(listener));
    UT(sum) == UT(0x1);

    sum = 0;
    t->Add("a1", 0x2, Properties::NodeOptions().Listener(listener));
    t->Add("b1", 0x4, Properties::NodeOptions().Listener(listener));
    t->Add("c1", 0x8, Properties::NodeOptions().Listener(listener));
    t->Add("a1/a2", 0x10, Properties::NodeOptions().Listener(listener));
    t->Add("a1/a2/a3", 0x20, Properties::NodeOptions().Listener(listener));
    t->Modify("x", 1);
    UT_THROWS(t->Commit(), Properties::InvalidOpException);
    UT(sum) == UT(0);

    t->Cancel();
    t->Add("a1", 0x2, Properties::NodeOptions().Listener(listener));
    t->Add("b1", 0x4, Properties::NodeOptions().Listener(listener));
    t->Add("c1", 0x8, Properties::NodeOptions().Listener(listener));
    t->Add("a1/a2", 0x10, Properties::NodeOptions().Listener(listener));
    t->Add("a1/a2/a3", 0x20, Properties::NodeOptions().Listener(listener));
    t->Commit();
    UT(sum) == UT(0x3f);

    sum = 0;
    props.Modify("a1/a2", 0x10);
    UT(sum) == UT(0x13);

    sum = 0;
    props.Modify("a1/a2/a3", 0x20);
    UT(sum) == UT(0x33);

    sum = 0;
    props.Delete("a1/a2");
    UT(sum) == UT(0x3);

    sum = 0;
    props.Add("a1/a2", 0x10, Properties::NodeOptions().Listener(listener));
    UT(sum) == UT(0x13);

    Properties::NodeHandlerConnection con;
    bool fired = false;
    Properties::NodeHandler listener2 =
        Properties::NodeHandler::Make([&fired](Properties::Node node) {

        UT_BOOL(node) == UT_TRUE;
        fired = true;
    }, std::placeholders::_1);
    bool changed = false;
    props.SignalChanged().Connect(Slot<void(Properties &)>::Make([&changed](){changed = true;}));

    sum = 0;
    props.Add("a1/a2/a3", 0x20,
              Properties::NodeOptions().Listener(listener).Listener(listener2, &con));
    UT(sum) == UT(0x33);
    UT(fired) == UT_TRUE;
    UT(changed) == UT_TRUE;

    sum = 0;
    fired = false;
    changed = false;
    props.Modify("a1/a2/a3", 0x40);
    UT(sum) == UT(0x53);
    UT(fired) == UT_TRUE;
    UT(changed) == UT_TRUE;

    con.Disconnect();
    sum = 0;
    fired = false;
    changed = false;
    props.Modify("a1/a2/a3", 0x20);
    UT(sum) == UT(0x33);
    UT(fired) == UT_FALSE;
    UT(changed) == UT_TRUE;
}
