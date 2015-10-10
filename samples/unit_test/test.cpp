/* This file is a part of ADK library.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See LICENSE file for copyright details.
 */

/** @file test.cpp
 * Sample unit test implementation.
 */

#include <adk_ut.h>

#include "component/component.h"

UT_TEST("Sample test")
{
    UT(SampleString()) == UT("Sample string");

    UT_CKPOINT("Multiplication");

    UT(SampleMult(5, 3)) == UT(15);
}
