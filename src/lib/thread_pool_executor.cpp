/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file thread_pool_executor.cpp */

#include <adk.h>

using namespace adk;

ThreadPoolExecutor::ThreadPoolExecutor(int numThreads, size_t queueSize):
    numThreads(numThreads), queue(queueSize)
{
    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back(&ThreadPoolExecutor::ThreadFunc, this);
    }
}

ThreadPoolExecutor::~ThreadPoolExecutor()
{
    Terminate();
}

void
ThreadPoolExecutor::Submit(const Action &action)
{
    queue.Push(action);
}

void
ThreadPoolExecutor::Submit(Action &&action)
{
    queue.Push(std::move(action));
}

void
ThreadPoolExecutor::WaitQueueEmpty()
{
    queue.WaitEmpty();
}

void
ThreadPoolExecutor::Terminate()
{
    queue.Exit();
    for (std::thread &t: threads) {
        t.join();
    }
    auto lock = queue.Lock();
    threads.clear();
}

void
ThreadPoolExecutor::ThreadFunc()
{
    while (!queue.IsExitRequested()) {
        Action action;
        if (queue.Pop(action)) {
            action();
        }
    }
}
