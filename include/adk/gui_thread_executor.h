/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See LICENSE file for copyright details.
 */

/** @file gui_thread_executor.h */

#ifndef GUI_THREAD_EXECUTOR_H_
#define GUI_THREAD_EXECUTOR_H_

namespace adk {

/** Executes action in GUI thread. Should be constructed in GUI thread. */
class GuiThreadExecutor: public Executor {
public:
    /** Create the executor.
     *
     * @param queueSize Maximal size of the actions queue. Zero for unlimited.
     *      Submission is blocked if the limit is reached.
     */
    GuiThreadExecutor(size_t queueSize = 0);

    void
    Submit(const Action &action) override;

    void
    Submit(Action &&action) override;

private:
    MessageQueue<Action> queue;
    Glib::Dispatcher dispatcher;

    /** Called in GUI thread after action submission. */
    void
    OnSubmit();
};

} /* namespace adk */

#endif /* GUI_THREAD_EXECUTOR_H_ */
