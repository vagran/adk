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
    props(props), haveButtons(haveButtons),
    wdgTlBox(Gtk::ORIENTATION_VERTICAL, 4),
    wdgButtonsBox(Gtk::ORIENTATION_HORIZONTAL, 4),
    wdgApplyButton("Apply"),
    wdgCancelButton("Cancel"),
    wdgValuesVp(Glib::RefPtr<Gtk::Adjustment>(), Glib::RefPtr<Gtk::Adjustment>())
{
    wdgValuesScrolled.add(wdgValuesVp);
    wdgDescScrolled.add(wdgDesc);

    wdgButtonsBox.set_homogeneous();
    wdgButtonsBox.set_size_request();
    wdgButtonsBox.pack_start(wdgApplyButton, true, false);
    wdgButtonsBox.pack_start(wdgCancelButton, true, false);

    wdgPaned.set_orientation(Gtk::ORIENTATION_VERTICAL);
    wdgPaned.add1(wdgValuesScrolled);
    wdgPaned.add2(wdgDescScrolled);

    wdgTlBox.pack_start(wdgPaned, true, true);
    wdgTlBox.pack_start(wdgButtonsBox, false, false);

    props.SignalChanged().Connect(Properties::ChangedHandler::Make(
        &PropView::OnPropsChanged, this));

    wdgApplyButton.signal_clicked().connect(
        sigc::mem_fun(*this, &PropView::OnApply));
    wdgCancelButton.signal_clicked().connect(
        sigc::mem_fun(*this, &PropView::OnCancel));
}

PropView::~PropView()
{
    //XXX
}

Gtk::Widget &
PropView::GetWidget()
{
    return wdgTlBox;
}

void
PropView::Show(bool f)
{
    if (f) {
        wdgTlBox.show();
        wdgPaned.show_all();
        if (haveButtons) {
            wdgButtonsBox.show_all();
        }
    } else {
        wdgTlBox.hide();
        wdgButtonsBox.hide();
    }
}

void
PropView::OnPropsChanged()
{
    //XXX
    ADK_INFO("props changed");
}

void
PropView::OnApply()
{
    //XXX
    ADK_INFO("applied");
}

void
PropView::OnCancel()
{
    //XXX
    ADK_INFO("cancel");
}
