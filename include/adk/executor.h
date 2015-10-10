/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See LICENSE file for copyright details.
 */

/** @file executor.h */

#ifndef EXECUTOR_H_
#define EXECUTOR_H_

namespace adk {

/** Base interface for executors. */
class Executor {
public:
    /** Callback to execute. */
    typedef std::function<void()> Action;

    /** Submit action for execution. */
    virtual void
    Submit(const Action &action) = 0;

    virtual void
    Submit(Action &&action)
    {
        /* Fall-back to copy version. */
        Submit(action);
    }
};

} /* namespace adk */

#endif /* EXECUTOR_H_ */
