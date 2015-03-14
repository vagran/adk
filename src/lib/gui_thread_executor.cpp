/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file gui_thread_executor.cpp */

#include <adk.h>

using namespace adk;

GuiThreadExecutor::GuiThreadExecutor(size_t queueSize):
    queue(queueSize)
{
    dispatcher.connect(sigc::mem_fun(*this, &GuiThreadExecutor::OnSubmit));
}

GuiThreadExecutor::~GuiThreadExecutor()
{
    //XXX
}

void
GuiThreadExecutor::Submit(const Action &action)
{
    queue.Push(action);
    dispatcher();
}

void
GuiThreadExecutor::Submit(Action &&action)
{
    queue.Push(std::move(action));
    dispatcher();
}

void
GuiThreadExecutor::OnSubmit()
{
    Action action;
    while (queue.TryPop(action)) {
        action();
    }
}
