/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
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
