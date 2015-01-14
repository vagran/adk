/* This file is a part of ADK library.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file adk.h
 * Main header file of ADK library. This is the only file which should be
 * included by application code. Include it first to speed up compilation time
 * by using pre-compiled header.
 */

#ifndef ADK_H_
#define ADK_H_

/* Python header should be the first because it can affect some global features. */
#ifdef ADK_USE_PYTHON
#include <Python.h>
#endif /* ADK_USE_PYTHON */

#include <adk/defs.h>

#ifdef ADK_PLATFORM_AVR

#include <adk/avr.h>

#else /* ADK_PLATFORM_AVR */
/* Desktop applications. */

#include <list>
#include <set>
#include <map>
#include <cstring>
#include <string>
#include <sstream>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

#include <glibmm.h>
#include <giomm.h>
#include <sigc++/sigc++.h>

#ifdef ADK_USE_GUI
#include <gtkmm.h>
#include <cairomm/cairomm.h>
#endif /* ADK_USE_GUI */

#ifdef UNITTEST
#include <adk_ut.h>
#endif

#include <adk/types.h>
#include <adk/endian.h>

#include <adk/static_init.h>

#include <adk/exception.h>
#include <adk/logging.h>
#include <adk/debug.h>
#include <adk/spinlock.h>

#include <adk/utils.h>
#include <adk/optional.h>
#include <adk/bitmap.h>
#include <adk/hash.h>
#include <adk/rb_tree.h>

#include <adk/signal.h>
#include <adk/resources.h>
#include <adk/xml.h>
#include <adk/properties.h>
#include <adk/locale.h>

#ifdef ADK_USE_GUI
#include <adk/glade.h>
#include <adk/prop_view.h>
#endif /* ADK_USE_GUI */

#ifdef ADK_USE_PYTHON
#include <adk/python.h>
#include <adk/python_ext.h>
#endif /* ADK_USE_PYTHON */

#endif /* ADK_PLATFORM_AVR */

#ifdef ADK_AVR_USE_USB
#include <adk/usb.h>
#endif /* ADK_AVR_USE_USB */

#endif /* ADK_H_ */
