/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file avr_cpp.h
 * C++ utilities for AVR platform.
 */

#ifndef AVR_CPP_H_
#define AVR_CPP_H_

namespace adk {

/** Disable interrupts in scope of this class instance. Restores previous state
 * when the scope is left.
 */
class AtomicSection {
public:
    AtomicSection():
        sreg(SREG)
    {
        cli();
    }

    ~AtomicSection()
    {
        SREG = sreg;
    }
private:
    u8 sreg;
};

} /* namespace adk */

#ifdef ADK_AVR_USE_COMMON_LIB
#include <adk/avr/scheduler.h>
#endif

#endif /* AVR_CPP_H_ */
