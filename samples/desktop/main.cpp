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
    Gtk::Main toolkit(argc, argv);
    Gtk::Window window;

    toolkit.run(window);
    return 0;
}
