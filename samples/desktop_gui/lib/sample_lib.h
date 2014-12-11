/* This file is a part of ADK library.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file sample_lib.h
 * Sample library.
 */

#ifndef SAMPLE_LIB_H_
#define SAMPLE_LIB_H_

class SampleLib {
public:
    SampleLib();
    ~SampleLib();
    Glib::RefPtr<Gtk::Builder> Test();
};

extern SampleLib g_sampleLib;

#endif /* SAMPLE_LIB_H_ */
