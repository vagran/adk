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
class PropView: public SlotTarget, virtual public sigc::trackable {

public:
    PropView(Properties &props, bool haveButtons = false);

    ~PropView();

    /** Get top-level widget for the properties sheet. */
    Gtk::Widget &
    GetWidget();

    /** Show or hide the properties sheet. */
    void
    Show(bool f = true);

private:
    //XXX
    class Node {

    };

    class Item: public Node {
    public:

    private:

    };

    class Category: public Node {

    };

    /** Associated properties. */
    Properties &props;
    /** Indicates whether the property sheet has apply and cancel buttons. */
    bool haveButtons;
    /** Top level box widget. */
    Gtk::Box wdgTlBox,
    /** Box for buttons. */
             wdgButtonsBox;
    /** Paned widget between values and description. */
    Gtk::Paned wdgPaned;
    /** Scrolling for description. */
    Gtk::ScrolledWindow wdgDescScrolled,
    /** Scrolling for values. */
                        wdgValuesScrolled;
    /** Description text. */
    Gtk::TextView wdgDesc;
    Gtk::Button wdgApplyButton, wdgCancelButton;
    /** Values viewport. */
    Gtk::Viewport wdgValuesVp;

    void
    OnPropsChanged();

    void
    OnApply();

    void
    OnCancel();
};

} /* namespace adk */

#endif /* PROP_VIEW_H_ */
