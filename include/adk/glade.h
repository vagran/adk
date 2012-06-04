/* /ADK/include/adk/glade.h
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file glade.h
 * Definitions related to Glade UI editor.
 */

#ifndef GLADE_H_
#define GLADE_H_

/* Automatically generated files for corresponding .glade files. */
#include <auto_adk_glade.h>

/** Start of embedded Glade XML. */
#define ADK_GLADE_XML_START(__fileName) &_binary_ ## __fileName ## _glade_start
/** End of embedded Glade XML. */
#define ADK_GLADE_XML_END(__fileName) &_binary_ ## __fileName ## _glade_end
/** Create UTF-8 glib string with Glade XML.
 * @param __fileName Name of .glade file.
 * For example, if file was named "main_window.glade" then this macro should be
 * used as in the example:
 * @code
 * Glib::RefPtr<Gtk::Builder> builder =
 *      Gtk::Builder::create_from_string(ADK_GLADE_XML(main_window));
 * @endcode
 */
#define ADK_GLADE_XML(__fileName) \
    Glib::ustring(ADK_GLADE_XML_START(__fileName), \
                  ADK_GLADE_XML_END(__fileName) - ADK_GLADE_XML_START(__fileName))

#endif /* GLADE_H_ */
