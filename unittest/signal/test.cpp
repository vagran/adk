/* /ADK/unittest/signal/test.cpp
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file test.cpp
 * Unit tests for signals and slots.
 */

#include <adk.h>
#include <adk_ut.h>

using namespace adk;

class NonTarget {
public:
    int
    Method(int a, int b)
    {
        return a + b;
    }

    int
    operator()(int a, int b)
    {
        return a + b;
    }

    static int
    StaticMethod(int a, int b)
    {
        return a + b;
    }
};

class Target: public SlotTarget {
public:
    int
    Method(int a, int b)
    {
        return a + b;
    }

    int
    operator()(int a, int b)
    {
        return a + b;
    }

    static int
    StaticMethod(int a, int b)
    {
        return a + b;
    }
};

int
Function(int a, int b)
{
    return a + b;
}

auto Lambda = [](int a, int b)
{
    return a + b;
};

UT_TEST("Basic functionality")
{
    Signal<int(int)> sig;
    Slot<int(int)> slot1, slot2, slot3;

    std::unique_ptr<NonTarget> nt(new NonTarget);
    std::unique_ptr<Target> t(new Target);

    UT((adk_internal::SlotTargetGetter<decltype(&Target::Method), Target *>::Get(t.get()))) == UT(t.get());
    UT((adk_internal::SlotTargetGetter<decltype(&NonTarget::Method), NonTarget *>::Get(nt.get()))) == UT_NULL;

    UT(adk_internal::GetSlotTarget(&Target::Method, t.get())) == UT(t.get());
    UT(adk_internal::GetSlotTarget(&NonTarget::Method, nt.get())) == UT_NULL;

    UT_BOOL(slot1) == UT_FALSE;
    UT_BOOL(slot2) == UT_FALSE;

    slot1 = Slot<int(int)>::Make(&NonTarget::Method, nt.get(), 10, std::placeholders::_1);
    slot2 = Slot<int(int)>::Make(&Target::Method, t.get(), 10, std::placeholders::_1);

    UT(slot1.GetTarget()) == UT_NULL;
    UT(slot2.GetTarget()) != UT_NULL;

    UT_BOOL(slot1) == UT_TRUE;
    UT_BOOL(slot2) == UT_TRUE;

    UT(slot1(20)) == UT(30);
    UT(slot2(30)) == UT(40);

    /* Target deletion should automatically unbind the slot. */
    nt = nullptr;
    t = nullptr;
    UT_BOOL(slot1) == UT_TRUE;
    UT_BOOL(slot2) == UT_FALSE;
}
