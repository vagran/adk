/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
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
        wdgCheck.signal_clicked().connect(
            sigc::mem_fun(*this, &PropView::Item::OnCheckbox));
    }

    wdgValue.signal_unmap().connect(
        sigc::mem_fun(*this, &PropView::Item::OnUnmap));
    wdgCheck.signal_unmap().connect(
        sigc::mem_fun(*this, &PropView::Item::OnUnmap));
}

void
PropView::Item::Update()
{
    wdgName.set_text(node.DispName());
    std::string desc(node.Description());
    if (desc.empty()) {
        desc = node.GetPath().Str();
    }
    wdgBox.set_tooltip_text(desc);
    UpdateValue();
}

void
PropView::Item::UpdateValue()
{
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
        std::string s(node.Val().Str());
        std::string units = node.Units();
        if (!units.empty()) {
            s += ' ';
            s += units;
        }
        wdgValue.set_text(s);
    }
}

bool
PropView::Item::OnFocusLost(GdkEventFocus *)
{
    if (!wdgValue.is_visible()) {
        return false;
    }

    auto ErrorMsg = [this](const std::string title, Properties::Exception &e) {
        std::string msg(node.GetPath().Str() + ":\n");
        msg += e.what();
        msg += "\nPress [Cancel] to restore original value, [OK] to correct your input.";
        Gtk::MessageDialog
            dlg(*dynamic_cast<Gtk::Window *>(wdgValue.get_toplevel()),
                msg, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK_CANCEL, true);
        dlg.set_title(title);
        if (dlg.run() == Gtk::RESPONSE_OK) {
            wdgValue.grab_focus();
            return true;
        }
        UpdateValue();
        return false;
    };

    Properties::Value v;
    try {
        v = Parse(wdgValue.get_text());
    } catch (Properties::ParseException &e) {
        return ErrorMsg("Value parsing failed", e);
    }
    if (v == node.Val()) {
        return false;
    }

    try {
        if (propView.hasButtons) {
            propView.trans->Modify(node.GetPath(), std::move(v));
            propView.transNodes.insert(this);
        } else {
            propView.props.Modify(node.GetPath(), std::move(v));
        }
    } catch (Properties::Exception &e) {
        return ErrorMsg("Value modification failed", e);
    }

    return false;
}

void
PropView::Item::OnCheckbox()
{
    auto ErrorMsg = [this](const std::string title, Properties::Exception &e) {
        std::string msg(node.GetPath().Str() + ":\n");
        msg += e.what();
        Gtk::MessageDialog
            dlg(*dynamic_cast<Gtk::Window *>(wdgValue.get_toplevel()),
                msg, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
        dlg.set_title(title);
        dlg.run();
        UpdateValue();
        return;
    };

    bool v = wdgCheck.get_active();
    if (v == node.Val<bool>()) {
        return;
    }

    try {
        if (propView.hasButtons) {
            propView.trans->Modify(node.GetPath(), v);
            propView.transNodes.insert(this);
        } else {
            propView.props.Modify(node.GetPath(), v);
        }
    } catch (Properties::Exception &e) {
        return ErrorMsg("Value modification failed", e);
    }
}

void
PropView::Item::OnUnmap()
{
    UpdateValue();
}

Properties::Value
PropView::Item::Parse(const std::string &s)
{
    std::string buf;
    /* Strip units if present. */
    std::string units = node.Units();
    buf = s;
    if (!units.empty()) {
        if (s.size() >= units.size()) {
            size_t unitsStart = s.size() - units.size();
            bool matched = true;
            for (size_t i = 0; i < units.size(); i++) {
                if (s[unitsStart + i] != units[i]) {
                    matched = false;
                    break;
                }
            }
            if (matched) {
                /* Strip also spaces between value and units. */
                for (size_t i = unitsStart; i > 0; i--) {
                    if (s[i - 1] == ' ') {
                        unitsStart = i - 1;
                    }
                }
                buf = s.substr(0, unitsStart);
            }
        }
    }

    return Properties::Value::FromString(node.Type(), buf);
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
    wdgExpander.set_label(node.DispName());
    std::string desc(node.Description());
    if (desc.empty()) {
        desc = node.GetPath().Str();
    }
    wdgExpander.set_tooltip_text(desc);
}

/* ****************************************************************************/

PropView::PropView(Properties &props, bool readOnly, bool hasButtons):
    props(props), readOnly(readOnly), hasButtons(hasButtons && !readOnly),
    wdgTlBox(Gtk::ORIENTATION_VERTICAL, 4),
    wdgButtonsBox(Gtk::ORIENTATION_HORIZONTAL, 4),
    wdgApplyButton("Apply"),
    wdgCancelButton("Cancel"),
    wdgValuesVp(Glib::RefPtr<Gtk::Adjustment>(), Glib::RefPtr<Gtk::Adjustment>())
{
    wdgValuesScrolled.add(wdgValuesVp);

    wdgButtonsBox.set_homogeneous();
    wdgButtonsBox.pack_start(wdgApplyButton, true, false);
    wdgButtonsBox.pack_start(wdgCancelButton, true, false);

    wdgTlBox.pack_start(wdgValuesScrolled, true, true);
    wdgTlBox.pack_start(wdgButtonsBox, false, false);

    root = new Category(*this);
    IndexNode(root);
    wdgValuesVp.add(*root->GetWidget());

    props.SignalChanged().Connect(Properties::ChangedHandler::Make(
        &PropView::OnPropsChanged, this));

    wdgApplyButton.signal_clicked().connect(
        sigc::mem_fun(*this, &PropView::OnApply));
    wdgCancelButton.signal_clicked().connect(
        sigc::mem_fun(*this, &PropView::OnCancel));

    if (hasButtons) {
        trans = props.OpenTransaction();
    }

    OnPropsChanged();
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
        wdgValuesScrolled.show_all();
        if (hasButtons) {
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
        if (root->node) {
            UpdateCategory(*root);
        }
    }
}

void
PropView::OnApply()
{
    try {
        trans->Commit();
    } catch (Properties::Exception &e) {
        std::string msg(e.what());
        msg += "\nPress [Cancel] to restore original values and cancel the transaction,\n"
               "[OK] to correct your input.";
        Gtk::MessageDialog
            dlg(*dynamic_cast<Gtk::Window *>(wdgTlBox.get_toplevel()),
                msg, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK_CANCEL, true);
        dlg.set_title("Values commit failed");
        if (dlg.run() == Gtk::RESPONSE_OK) {
            return;
        }
        trans->Cancel();
        for (Node *node: transNodes) {
            dynamic_cast<Item *>(node)->UpdateValue();
        }
    }
    transNodes.clear();
}

void
PropView::OnCancel()
{
    trans->Cancel();
    for (Node *node: transNodes) {
        dynamic_cast<Item *>(node)->UpdateValue();
    }
    transNodes.clear();
}

void
PropView::UpdateCategory(Category &catNode)
{
    catNode.Update();

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
            catNode.wdgList.add(*vnode->GetWidget());

            props.Modify(node.GetPath(), Properties::NodeOptions().Listener(
                Properties::NodeHandler::Make(&PropView::OnNodeChanged,
                                              this, vnode, std::placeholders::_2)));
        }
        order++;
    }
    catNode.wdgList.show_all();

    /* Delete non visited. */
    for (auto it = catNode.children.begin(); it != catNode.children.end();) {
        Node *node = *it;
        if (node->order == -1) {
            DeleteNode(catNode, node);
            it = catNode.children.erase(it);
            UnindexNode(node);
        } else {
            it++;
        }
    }
}

void
PropView::DeleteNode(Category &parent, Node *node)
{
    parent.wdgList.remove(*node->GetWidget()->get_parent());
    if (node->IsItem()) {
        return;
    }
    Category &cat = node->GetCategory();
    for (auto it = cat.children.begin(); it != cat.children.end();) {
        Node *node = *it;
        DeleteNode(cat, node);
        it = cat.children.erase(it);
        UnindexNode(node);
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
    auto transIt = transNodes.find(node);
    if (transIt != transNodes.end()) {
        transNodes.erase(transIt);
    }

    auto it = nodes.find(node->GetWidget());
    if (it == nodes.end()) {
        return nullptr;
    }
    std::unique_ptr<Node> ref = std::move(it->second);
    nodes.erase(it);
    return ref;
}

void
PropView::OnNodeChanged(Node *node, int event)
{
    if (node->IsItem() && (event & Properties::EventType::MODIFY)) {
        node->Update();
    } else if (!node->IsItem() &&
               (event & (Properties::EventType::NEW |
                         Properties::EventType::ADD |
                         Properties::EventType::DELETE |
                         Properties::EventType::MODIFY))) {
        UpdateCategory(node->GetCategory());
    }
}
