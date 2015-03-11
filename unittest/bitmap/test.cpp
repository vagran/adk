/* This file is a part of ADK library.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file test.cpp
 * Unit tests for Bitmap class.
 */

#include <adk.h>
#include <adk_ut.h>

using namespace adk;

const size_t numBits = 33;

/* Check first n bits are set. */
template <class BitmapT>
void
CheckFirstSet(BitmapT &bm, size_t n)
{
    if (n == numBits) {
        UT(bm.FirstClear()) == UT_SIZE(-1);
        UT(bm.FirstSet()) == UT_SIZE(0);
    } else {
        if (n == 0) {
            UT(bm.FirstSet()) == UT_SIZE(-1);
        }
        UT(bm.FirstClear()) == UT(n);
    }
    for (size_t i = 0; i < numBits; i++) {
        if (i < n) {
            UT(bm.IsSet(i)) == UT_TRUE;
            UT(bm.IsClear(i)) == UT_FALSE;
        } else {
            UT(bm.IsSet(i)) == UT_FALSE;
            UT(bm.IsClear(i)) == UT_TRUE;
        }
    }
}

/* Check first n bits are cleared. */
template <class BitmapT>
void
CheckFirstClear(BitmapT &bm, size_t n)
{
    if (n == numBits) {
        UT(bm.FirstSet()) == UT_SIZE(-1);
        UT(bm.FirstClear()) == UT_SIZE(0);
    } else {
        if (n == 0) {
            UT(bm.FirstClear()) == UT_SIZE(-1);
        }
        UT(bm.FirstSet()) == UT(n);
    }
    for (size_t i = 0; i < numBits; i++) {
        if (i < n) {
            UT(bm.IsClear(i)) == UT_TRUE;
            UT(bm.IsSet(i)) == UT_FALSE;
        } else {
            UT(bm.IsClear(i)) == UT_FALSE;
            UT(bm.IsSet(i)) == UT_TRUE;
        }
    }
}

template <class BitmapT>
void
TestBitmap(BitmapT &bm)
{
    CheckFirstSet(bm, 0);
    CheckFirstClear(bm, numBits);
    for (size_t i = 0; i < numBits; i++) {
        bm.Set(i);
        CheckFirstSet(bm, i + 1);
        bm.Invert();
        CheckFirstClear(bm, i + 1);
        bm.Invert();
        CheckFirstSet(bm, i + 1);
    }
    CheckFirstClear(bm, 0);
    CheckFirstSet(bm, numBits);
    for (size_t i = 0; i < numBits; i++) {
        bm.Clear(i);
        CheckFirstClear(bm, i + 1);
        bm.Invert();
        CheckFirstSet(bm, i + 1);
        bm.Invert();
        CheckFirstClear(bm, i + 1);
    }
}

UT_TEST("Static bitmap")
{
    Bitmap<numBits> bm;
    TestBitmap(bm);
}

UT_TEST("Dynamic bitmap")
{
    Bitmap<-1> bm(numBits);
    TestBitmap(bm);
}
