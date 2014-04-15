/* /ADK/include/adk/prop_view.h
 *
 * This file is a part of 'ADK' project.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file prop_view.h
 * Properties view class declaration.
 */

#ifndef PROP_VIEW_H_
#define PROP_VIEW_H_

namespace adk {

/** View for properties sheet. Used in GUI applications for displaying and
 * changing properties.
 */
class PropView: public SlotTarget {

public:
    PropView(Properties &props, bool haveButtons = false);

    ~PropView();

    /** Get top-level widget for the properties sheet. */
    Gtk::Widget *
    GetWidget();

private:
    /** Associated properties. */
    Properties &props;
    /** Indicates whether the property sheet has apply and cancel buttons. */
    bool haveButtons;
    /** Top level widget. */
    Gtk::Label *tlWidget;//XXX

    void
    OnPropsChanged();
};

} /* namespace adk */

#endif /* PROP_VIEW_H_ */
