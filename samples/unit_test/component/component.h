/* /ADK/samples/unit_test/component/component.h
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file component.h
 * Interface of sample component being tested.
 */

#ifndef COMPONENT_H_
#define COMPONENT_H_

const char *
SampleString();

int
SampleMult(int a, int b);

/** This function is not used in tests but it references external function
 * SampleExtern which is not defined in this module. The framework will
 * generate automatic stub for the external function to resolve linking.
 */
void
SampleUnused();

void
SampleExtern();

#endif /* COMPONENT_H_ */
