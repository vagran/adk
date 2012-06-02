/* /ADK/include/adk.h
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file adk.h
 * TODO insert description here.
 */

#ifndef ADK_H_
#define ADK_H_

#ifdef ADK_PLATFORM_AVR

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#else /* ADK_PLATFORM_AVR */
/* Desktop applications. */

#include <gtkmm.h>
#include <cairomm/cairomm.h>

#endif /* ADK_PLATFORM_AVR */

#endif /* ADK_H_ */
