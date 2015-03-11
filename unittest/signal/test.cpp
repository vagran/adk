/* This file is a part of ADK library.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
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
    int lastResult = 0;

    int
    Method(int a, int b)
    {
        lastResult = a + b;
        return a + b;
    }

    int
    operator()(int a, int b)
    {
        lastResult = a + b;
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
    int lastResult = 0;

    int
    Method(int a, int b)
    {
        lastResult = a + b;
        return a + b;
    }

    int
    operator()(int a, int b)
    {
        lastResult = a + b;
        return a + b;
    }

    static int
    StaticMethod(int a, int b)
    {
        return a + b;
    }
};

class Callable {
public:
    int x;
    int lastResult = 0;

    Callable(int x): x(x) {}

    int
    operator()(int a, int b)
    {
        lastResult = a + b;
        return a + b + x;
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
    slot3 = slot2;
    UT(slot3(30)) == UT(40);

    auto con1 = sig.Connect(slot1);
    auto con2 = sig.Connect(slot2);
    auto con3 = SignalProxy<int(int)>(sig).Connect(slot3);

    nt->lastResult = 0;
    t->lastResult = 0;
    sig.Emit(20);
    UT(nt->lastResult) == UT(30);
    UT(t->lastResult) == UT(30);

    con1.Disconnect();
    nt->lastResult = 0;
    t->lastResult = 0;
    sig.EmitNoResult(20);
    UT(nt->lastResult) == UT(0);
    UT(t->lastResult) == UT(30);

    con1 = sig.Connect(slot1);
    nt->lastResult = 0;
    t->lastResult = 0;
    sig.EmitNoResult(20);
    UT(nt->lastResult) == UT(30);
    UT(t->lastResult) == UT(30);

    /* Target deletion should automatically unbind the slot. */
    t = nullptr;
    UT_BOOL(slot1) == UT_TRUE;
    UT_BOOL(slot2) == UT_FALSE;
    UT_BOOL(slot3) == UT_FALSE;

    sig.Emit(20);

    UT_BOOL(con1) == UT_TRUE;
    UT_BOOL(con2) == UT_FALSE;
    UT_BOOL(con3) == UT_FALSE;

    nt = nullptr;
    UT_BOOL(slot1) == UT_TRUE;

    con1.Disconnect();
    UT_BOOL(con1) == UT_FALSE;
    sig.Emit(20);

    /* Lambda target. */
    auto target = [](int a, int b) { return a + b + 10; };
    slot1 = Slot<int(int)>::Make(target, 10, std::placeholders::_1);
    UT(slot1(20)) == UT(40);

    /* Callable object. */
    Callable c(15);
    slot1 = Slot<int(int)>::Make(c, 10, std::placeholders::_1);
    UT(slot1(20)) == UT(45);
}

UT_TEST("Signal deletion")
{
    std::unique_ptr<Signal<int(int)>> sig(new Signal<int(int)>);
    Slot<int(int)> slot1, slot2;

    std::unique_ptr<NonTarget> nt(new NonTarget);
    std::unique_ptr<Target> t(new Target);

    slot1 = Slot<int(int)>::Make(&NonTarget::Method, nt.get(), 10, std::placeholders::_1);
    slot2 = Slot<int(int)>::Make(&Target::Method, t.get(), 10, std::placeholders::_1);

    auto con1 = sig->Connect(slot1);
    auto con2 = sig->Connect(slot2);

    sig->Emit(20);

    sig = nullptr;
    UT_BOOL(con1) == UT_FALSE;
    UT_BOOL(con2) == UT_FALSE;
}
