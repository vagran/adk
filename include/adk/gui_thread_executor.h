/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file gui_thread_executor.h */

#ifndef GUI_THREAD_EXECUTOR_H_
#define GUI_THREAD_EXECUTOR_H_

namespace adk {

/** Executes action in GUI thread. Should be constructed in GUI thread. */
class GuiThreadExecutor {
public:
    typedef std::function<void()> Action;

    GuiThreadExecutor(size_t queueSize);

    ~GuiThreadExecutor();

    void
    Submit(const Action &action);

    void
    Submit(Action &&action);

private:
    MessageQueue<Action> queue;
    Glib::Dispatcher dispatcher;

    /** Called in GUI thread after action submission. */
    void
    OnSubmit();
};

} /* namespace adk */

#endif /* GUI_THREAD_EXECUTOR_H_ */
