/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file optional.cpp
 * Tests for Optional class.
 */

#include <adk.h>
#include <adk_ut.h>

using namespace adk;

class C {
public:
    int x;

    C(int x):
        x(x)
    {}

    C(C &&c):
        x(c.x)
    {
        c.x = 0;
    }

    C(const C &) = default;

    C &
    operator =(const C &) = default;
    C &
    operator =(C &&) = default;
};

UT_TEST("Optional class")
{
    {
        Optional<C> x;
        UT_BOOL(x) == UT_FALSE;

        x = C(2);
        UT_BOOL(x) == UT_TRUE;
        UT(x->x) == UT(2);

        x = nullopt;
        UT_BOOL(x) == UT_FALSE;
    }

    {
        Optional<C> x(1);
        UT_BOOL(x) == UT_TRUE;
        UT(x->x) == UT(1);

        Optional<C> y(2);
        x = y;
        UT(x->x) == UT(2);

        x = nullopt;
        UT_BOOL(x) == UT_FALSE;

        x = std::move(y);
        UT(x->x) == UT(2);
        UT(y->x) == UT(0);
    }

    {
        C c(2);
        Optional<C> x(1);
        x = c;
        UT(x->x) == UT(2);

        x = nullopt;
        UT_BOOL(x) == UT_FALSE;

        x = std::move(c);
        UT(x->x) == UT(2);
        UT(c.x) == UT(0);

        x = 3;
        UT(x->x) == UT(3);
    }
}
