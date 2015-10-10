/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See LICENSE file for copyright details.
 */

/** @file optional.cpp
 * Tests for Optional class.
 */

#include <adk.h>
#include <adk_ut.h>

using namespace adk;

UT_TEST("Message queue")
{
    MessageQueue<int> queue;

    volatile int lastItem = 0;
    volatile int numItems = 0;
    auto thread = std::thread([&](){
        while (numItems < 2 || !queue.IsExitRequested()) {
            int item;
            if (queue.Pop(item)) {
                lastItem = item;
                numItems++;
            }
        }
    });

    queue.Push(10);
    queue.Push(42);
    queue.Exit();
    thread.join();
    UT(lastItem) == UT(42);
    UT(numItems) == UT(2);
}

UT_TEST("Thread pool executor")
{
    ThreadPoolExecutor ex(5);

    std::atomic<int> counter(0);

    for (int i = 0; i < 2000; i++) {
        ex.Submit([&](){ counter++; });
    }
    ex.WaitQueueEmpty();
    ex.Terminate();
    UT(counter.load()) == UT(2000);
}
