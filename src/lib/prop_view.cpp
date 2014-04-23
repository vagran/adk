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

PropView::Item::Item(PropView &propView):
    Node(propView),
    wdgBox(Gtk::ORIENTATION_HORIZONTAL, 2),
    wdgNameAlign(1.0, 0.5, 0.0, 1.0)
{
    wdgBox.set_homogeneous();
    wdgNameAlign.add(wdgName);
    wdgBox.pack_start(wdgNameAlign, true, true);
    wdgBox.pack_end(wdgValue, true, true);

    if (propView.readOnly) {
        wdgValue.set_editable(false);
        wdgCheck.set_sensitive(false);
    } else {
        wdgValue.signal_focus_out_event().connect(
            sigc::mem_fun(*this, &PropView::Item::OnFocusLost));
    }

    wdgValue.signal_unmap().connect(
        sigc::mem_fun(*this, &PropView::Item::OnHide));
}

void
PropView::Item::Update()
{
    wdgName.set_text(node.DispName());
    if (node.Type() == Properties::Value::Type::BOOLEAN) {
        if (isText) {
            wdgBox.remove(wdgValue);
            wdgBox.pack_end(wdgCheck, true, true);
            isText = false;
        }
        wdgCheck.set_active(node.Val<bool>());
    } else {
        if (!isText) {
            wdgBox.remove(wdgCheck);
            wdgBox.pack_end(wdgValue, true, true);
            isText = true;
        }
        wdgValue.set_text(node.Val().Str());//XXX
    }
}

bool
PropView::Item::OnFocusLost(GdkEventFocus *)
{
    if (!wdgValue.is_visible()) {
        return false;
    }
    wdgValue.grab_focus();
    return true;
}

void
PropView::Item::OnHide()
{
    //restore value if editing
}

/* ****************************************************************************/

PropView::Category::Category(PropView &propView):
    Node(propView)
{
    wdgList.set_sort_func(sigc::mem_fun(propView, &PropView::CategorySortFunc));
    wdgExpander.add(wdgList);
}

PropView::Node *
PropView::Category::FindChild(Properties::Node node)
{
    for (Node *nodePtr: children) {
        if (nodePtr->node == node) {
            return nodePtr;
        }
    }
    return nullptr;
}

void
PropView::Category::Update()
{
    //XXX
}

/* ****************************************************************************/

PropView::PropView(Properties &props, bool readOnly, bool haveButtons):
    props(props), readOnly(readOnly), haveButtons(haveButtons && !readOnly),
    wdgTlBox(Gtk::ORIENTATION_VERTICAL, 4),
    wdgButtonsBox(Gtk::ORIENTATION_HORIZONTAL, 4),
    wdgApplyButton("Apply"),
    wdgCancelButton("Cancel"),
    wdgValuesVp(Glib::RefPtr<Gtk::Adjustment>(), Glib::RefPtr<Gtk::Adjustment>())
{
    wdgDesc.set_editable(false);
    wdgValuesScrolled.add(wdgValuesVp);
    wdgDescScrolled.add(wdgDesc);

    wdgButtonsBox.set_homogeneous();
    wdgButtonsBox.pack_start(wdgApplyButton, true, false);
    wdgButtonsBox.pack_start(wdgCancelButton, true, false);

    wdgPaned.set_orientation(Gtk::ORIENTATION_VERTICAL);
    wdgPaned.add1(wdgValuesScrolled);
    wdgPaned.add2(wdgDescScrolled);

    wdgTlBox.pack_start(wdgPaned, true, true);
    wdgTlBox.pack_start(wdgButtonsBox, false, false);
    wdgTlBox.set_size_request(-1, 200);

    root = new Category(*this);
    IndexNode(root);
    wdgValuesVp.add(*root->GetWidget());

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
    if (!root->node) {
        root->node = props[""];
        UpdateCategory(*root);
    }
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

void
PropView::UpdateCategory(Category &catNode)
{
    catNode.wdgExpander.set_label(catNode.node.DispName());

    for (Node *node: catNode.children) {
        node->order = -1;
    }

    int order = 0;
    for (Properties::Node node: catNode.node) {
        Node *vnode = catNode.FindChild(node);
        if (vnode) {
            vnode->order = order;
        } else {
            /* New node. */
            auto value = node.Val();
            if (value.IsNone()) {
                vnode = new Category(*this);
            } else {
                vnode = new Item(*this);
            }
            catNode.children.push_back(vnode);
            IndexNode(vnode);
            vnode->node = node;
            vnode->order = order;
            if (value.IsNone()) {
                UpdateCategory(vnode->GetCategory());
            } else {
                vnode->Update();
            }
            catNode.wdgList.add(*vnode->GetWidget());
        }
        order++;
    }

    /* Delete non visited. */
    for (auto it = catNode.children.begin(); it != catNode.children.end();) {
        Node *node = *it;
        if (node->order == -1) {
            catNode.wdgList.remove(*node->GetWidget());
            it = catNode.children.erase(it);
            UnindexNode(node);
        } else {
            it++;
        }
    }
}

int
PropView::CategorySortFunc(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2)
{
    Node *node1 = nodes[row1->get_child()].get();
    Node *node2 = nodes[row2->get_child()].get();
    return node1->order - node2->order;
}

void
PropView::IndexNode(Node *node)
{
    nodes.emplace(node->GetWidget(), std::unique_ptr<Node>(node));
}

std::unique_ptr<PropView::Node>
PropView::UnindexNode(Node *node)
{
    auto it = nodes.find(node->GetWidget());
    if (it == nodes.end()) {
        return nullptr;
    }
    std::unique_ptr<Node> ref = std::move(it->second);
    nodes.erase(it);
    return ref;
}
