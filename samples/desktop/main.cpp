/* /ADK/samples/desktop/main.cpp
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file main.cpp
 * TODO insert description here.
 */

#include <adk.h>

int
main(int argc, char **argv)
{
    Gtk::Main app(argc, argv);
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file("sample.glade");
    Gtk::Window *window = 0;
    builder->get_widget("main_wnd", window);
    app.run(*window);
    delete window;
    return 0;
}
