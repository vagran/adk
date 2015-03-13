/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file thread_pool_executor.h */

#ifndef THREAD_POOL_EXECUTOR_H_
#define THREAD_POOL_EXECUTOR_H_

namespace adk {

class ThreadPoolExecutor {
public:
    typedef std::function<void()> Action;

    /** Create the executor.
     *
     * @param numThreads Maximal number of threads to create.
     * @param queueSize Queue size. Zero is unlimited. Submit blocks when limit
     *      reached.
     */
    ThreadPoolExecutor(int numThreads = 1, size_t queueSize = 0);

    ~ThreadPoolExecutor();

    /** Submit action for execution in the worker threads. */
    void
    Submit(const Action &action);

    /** Submit action for execution in the worker threads. */
    void
    Submit(Action &&action);

    /** Wait until pending actions queue is empty. This however does not
     * guarantee that the peeked tasks are already completed.
     */
    void
    WaitQueueEmpty();

    /** Terminate all threads. This is done automatically during destruction. */
    void
    Terminate();

private:
    int numThreads;
    std::vector<std::thread> threads;
    MessageQueue<Action> queue;

    void
    ThreadFunc();
};

}

#endif /* THREAD_POOL_EXECUTOR_H_ */
