/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file message_queue.cpp */

#include <adk.h>

using namespace adk;
using namespace adk::internal;

MessageQueueBase::QueueLock
MessageQueueBase::Lock()
{
    return QueueLock(mutex);
}

void
MessageQueueBase::Exit(QueueLock &&lock)
{
    if (!lock) {
        lock = QueueLock(mutex);
    }
    exit = true;
    cvPop.notify_all();
    cvPush.notify_all();
}

/** Check if exit was requested. */
bool
MessageQueueBase::IsExitRequested(QueueLock &&lock)
{
    if (!lock) {
        lock = QueueLock(mutex);
    }
    return exit;
}

MessageQueueBase::QueueLock
MessageQueueBase::AcquirePush(QueueLock &&lock)
{
    if (exit) {
        return QueueLock();
    }

    if (maxLen != 0 && curLen >= maxLen) {
        cvPush.wait(lock, [this](){ return exit || curLen < maxLen; });
    }
    if (exit) {
        return QueueLock();
    }
    return std::move(lock);
}

MessageQueueBase::QueueLock
MessageQueueBase::AcquirePop(QueueLock &&lock)
{
    if (exit) {
        if (curLen == 0) {
            return QueueLock();
        }
        return std::move(lock);
    }
    if (curLen == 0) {
        cvPop.wait(lock, [this](){ return exit || curLen != 0; });
    }
    if (curLen != 0) {
        return std::move(lock);
    }
    return QueueLock();
}

MessageQueueBase::QueueLock
MessageQueueBase::AcquirePop(QueueLock &&lock, const std::chrono::milliseconds &timeout)
{
    if (exit) {
        if (curLen == 0) {
            return QueueLock();
        }
        return std::move(lock);
    }
    if (curLen == 0) {
        if (!cvPop.wait_for(lock, timeout, [this](){ return exit || curLen != 0; })) {
            return QueueLock();
        }
    }
    if (curLen != 0) {
        return std::move(lock);
    }
    return QueueLock();
}

void
MessageQueueBase::CommitPush(QueueLock &&lock)
{
    curLen++;
    lock.unlock();
    cvPop.notify_one();
}

void
MessageQueueBase::CommitPop(QueueLock &&lock)
{
    bool needNotification = (maxLen != 0 && curLen == maxLen) || curLen == 1;
    curLen--;
    lock.unlock();
    if (needNotification) {
        cvPush.notify_all();
    }
}

void
MessageQueueBase::WaitEmpty(QueueLock &&lock)
{
    cvPush.wait(lock, [this](){ return curLen == 0; });
}

bool
MessageQueueBase::WaitEmpty(QueueLock &&lock, const std::chrono::milliseconds &timeout)
{
    return cvPush.wait_for(lock, timeout, [this](){ return curLen == 0; });
}
