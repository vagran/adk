/* This file is a part of ADK library.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file glade.h
 * Definitions related to Glade UI editor.
 */

#ifndef ADK_GLADE_H_
#define ADK_GLADE_H_

namespace adk {

/** Create UTF-8 glib string with Glade XML.
 * @param __fileName Name of .glade file.
 * For example, if file was named "main_window.glade" then this macro should be
 * used as in the example:
 * @code
 * Glib::RefPtr<Gtk::Builder> builder =
 *      Gtk::Builder::create_from_string(ADK_GLADE_XML(main_window));
 * @endcode
 */
#define ADK_GLADE_XML(__fileName) ({ \
    adk::ResourceDesc res = adk::GetResource(__STR(__fileName) ".glade"); \
    Glib::ustring(static_cast<const char *>(res.GetData()), res.GetSize()); \
})

} /* namespace adk */

#endif /* ADK_GLADE_H_ */
