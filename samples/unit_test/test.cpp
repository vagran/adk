/* /ADK/samples/unit_test/test.cpp
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file test.cpp
 * Sample unit test implementation.
 */

#include <adk_ut.h>

#include "component/component.h"

UT_TEST("Sample test")
{
    UT(SampleString()) == UT_CSTR("Sample string");
    UT(SampleMult(5, 3)) == UT(15);
}
UT_TEST_END
