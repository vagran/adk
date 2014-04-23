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
    PropView(Properties &props, bool readOnly = false, bool hasButtons = false);

    ~PropView();

    /** Get top-level widget for the properties sheet. */
    Gtk::Widget &
    GetWidget();

    /** Show or hide the properties sheet. */
    void
    Show(bool f = true);

private:
    class Item;
    class Category;

    class Node: public SlotTarget, virtual public sigc::trackable {
    public:
        PropView &propView;
        Properties::Node node;
        /** Order value, nodes in list sorted by this  value. */
        int order;

        Node(PropView &propView):
            propView(propView)
        {}

        virtual
        ~Node()
        {}

        virtual bool
        IsItem() const = 0;

        /** Update view. */
        virtual void
        Update() = 0;

        /** Get main widget. */
        virtual Gtk::Widget *
        GetWidget() = 0;

        Item &
        GetItem()
        {
            ASSERT(IsItem());
            return dynamic_cast<Item &>(*this);
        }

        Category &
        GetCategory()
        {
            ASSERT(!IsItem());
            return dynamic_cast<Category &>(*this);
        }
    };

    class Item: public Node {
    public:
        Gtk::Box wdgBox;
        Gtk::Alignment wdgNameAlign;
        Gtk::Label wdgName;
        Gtk::Entry wdgValue;
        Gtk::CheckButton wdgCheck;
        bool isText = true;

        Item(PropView &propView);

        virtual bool
        IsItem() const override
        {
            return true;
        }

        virtual void
        Update() override;

        virtual Gtk::Widget *
        GetWidget() override
        {
            return &wdgBox;
        }

    private:
        bool
        OnFocusLost(GdkEventFocus *);

        void
        OnUnmap();

        void
        UpdateValue();

        /** Parse value from string (may include units).
         * @throws Properties::ParseException if parsing fails.
         */
        Properties::Value
        Parse(const std::string &s);
    };

    class Category: public Node {
    public:
        Gtk::Expander wdgExpander;
        Gtk::ListBox wdgList;
        std::list<Node *> children;

        Category(PropView &propView);

        virtual bool
        IsItem() const override
        {
            return false;
        }

        virtual void
        Update() override;

        virtual Gtk::Widget *
        GetWidget() override
        {
            return &wdgExpander;
        }

        Node *
        FindChild(Properties::Node node);
    };

    /** Nodes indexed by their primary widget pointer. For category nodes it is
     * expander widget, for items - box widget.
     */
    std::map<Gtk::Widget *, std::unique_ptr<Node>> nodes;

    /** Root category. */
    Category *root;

    /** Associated properties. */
    Properties &props;
    /** Active transaction when using buttons. */
    Properties::Transaction::Ptr trans;
    /** Indicates whether modifications are allowed by user. */
    bool readOnly;
    /** Indicates whether the property sheet has apply and cancel buttons. */
    bool hasButtons;
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

    /** Synchronize category contents with current view. */
    void
    UpdateCategory(Category &catNode);

    void
    IndexNode(Node *node);

    /** Remove node from index and return the pointer holding the last
     * reference to the node object.
     */
    std::unique_ptr<Node>
    UnindexNode(Node *node);

    int
    CategorySortFunc(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2);
};

} /* namespace adk */

#endif /* PROP_VIEW_H_ */
