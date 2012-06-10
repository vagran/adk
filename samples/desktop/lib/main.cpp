/* /ADK/samples/desktop/lib/main.cpp
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#include <adk.h>

#include "sample_lib.h"

SampleLib g_sampleLib;

SampleLib::SampleLib()
{
    g_message("SampleLib::SampleLib");
}

SampleLib::~SampleLib()
{
    g_message("SampleLib::~SampleLib");
}

Glib::RefPtr<Gtk::Builder>
SampleLib::Test()
{
    g_message("SampleLib::Test");
    return Gtk::Builder::create_from_string(ADK_GLADE_XML(main_window));
}