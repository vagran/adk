/* /ADK/samples/unit_test/component/main.cpp
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file main.cpp
 * File with implementation of sample component being test.
 */

#include "component.h"

const char *
SampleString()
{
    return "Sample string";
}

int
SampleMult(int a, int b)
{
    return a * b;
}

void
SampleUnused()
{
    SampleExtern();
}
