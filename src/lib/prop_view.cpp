/* /ADK/src/lib/prop_view.cpp
 *
 * This file is a part of 'ADK' project.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file prop_view.cpp
 * PropView class implementation.
 */

#include <adk.h>

using namespace adk;

PropView::PropView(Properties &props, bool haveButtons):
    props(props), haveButtons(haveButtons)
{
    props.SignalChanged().Connect(Properties::ChangedHandler::Make(
        &PropView::OnPropsChanged, this));
    tlWidget = new Gtk::Label("test");
}

PropView::~PropView()
{
    delete tlWidget;
}

Gtk::Widget *
PropView::GetWidget()
{
    return tlWidget;
}

void
PropView::OnPropsChanged()
{
    //XXX
    ADK_INFO("props changed");
}
