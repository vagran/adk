/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file message_queue.h */

#ifndef MESSAGE_QUEUE_H_
#define MESSAGE_QUEUE_H_

namespace adk {

namespace internal {

class MessageQueueBase {
public:
    typedef std::unique_lock<std::mutex> QueueLock;

    MessageQueueBase(size_t maxLen):
        maxLen(maxLen)
    {}

    /** Acquire queue lock. Can be passed to queue methods. */
    QueueLock Lock();

    /** Request exit. */
    void
    Exit(QueueLock &&lock = QueueLock());

    /** Check if exit was requested. */
    bool
    IsExitRequested(QueueLock &&lock = QueueLock());

protected:
    size_t maxLen, curLen = 0;
    std::mutex mutex;
    /** CV for pop waiting. */
    std::condition_variable cvPop,
    /** CV for push waiting. */
                            cvPush;
    bool exit = false;

    /** Acquire one element pushing. @ref CommitPush() should be called after
     * an element pushing.
     *  @return Valid lock if acquired, empty lock if not acquired.
     */
    QueueLock
    AcquirePush(QueueLock &&lock);


    /** Acquire one element popping. @ref CommitPop() should be called after
     * an element popping.
     *  @return Valid lock if acquired, empty lock if not acquired.
     */
    QueueLock
    AcquirePop(QueueLock &&lock);

    /** Acquire one element pushing.
     *  @return Valid lock if acquired, empty lock if not acquired.
     */
    QueueLock
    AcquirePop(QueueLock &&lock, const std::chrono::milliseconds &timeout);

    /** Commit pushing after push slot acquired by @ref AcquirePush(). */
    void
    CommitPush(QueueLock &&lock);

    /** Commit popping after push slot acquired by @ref AcquirePush(). */
    void
    CommitPop(QueueLock &&lock);

    /** Wait until queue is empty. */
    void
    WaitEmpty(QueueLock &&lock);

    /** Wait until queue is empty, or timeout expires. */
    bool
    WaitEmpty(QueueLock &&lock, const std::chrono::milliseconds &timeout);
};

} /* namespace internal */

/** Queue for inter-thread communications.
 * @param T Message type. Should be move and copy constructible and assignable.
 */
template<class T>
class MessageQueue: public internal::MessageQueueBase {
public:
    /** Construct the message queue.
     *
     * @param maxLen Maximal queue size. Pushing blocks after the limit is
     *      reached. Zero is no limit.
     */
    MessageQueue(size_t maxLen = 0):
        internal::MessageQueueBase(maxLen)
    {}

    /** Push message into the queue. Blocks if queue size limit reached.
     * @param msg Message to push.
     * @param lock Lock held if any. Should be previously acquired by @ref Lock()
     *      method.
     * @return true if message pushed, false otherwise (e.g. exit requested).
     */
    bool
    Push(const T &msg, QueueLock &&lock = QueueLock())
    {
        if (!lock) {
            lock = QueueLock(mutex);
        }
        lock = AcquirePush(std::move(lock));
        if (!lock) {
            return false;
        }
        queue.push(msg);
        CommitPush(std::move(lock));
        return true;
    }

    /** Push message into the queue. Blocks if queue size limit reached.
     * @param msg Message to push.
     * @param lock Lock held if any. Should be previously acquired by @ref Lock()
     *      method.
     * @return true if message pushed, false otherwise (e.g. exit requested).
     */
    bool
    Push(T &&msg, QueueLock &&lock = QueueLock())
    {
        if (!lock) {
            lock = QueueLock(mutex);
        }
        lock = AcquirePush(std::move(lock));
        if (!lock) {
            return false;
        }
        queue.emplace(std::move(msg));
        CommitPush(std::move(lock));
        return true;
    }

    /** Pop message from queue. Blocks until the message is available.
     * @param msg Message object to assign.
     * @param lock Lock held if any. Should be previously acquired by @ref Lock()
     *      method.
     * @return true if message popped, false otherwise (e.g. exit requested).
     */
    bool
    Pop(T &msg, QueueLock &&lock = QueueLock())
    {
        if (!lock) {
            lock = QueueLock(mutex);
        }
        lock = AcquirePop(std::move(lock));
        if (!lock) {
            return false;
        }
        msg = queue.front();
        queue.pop();
        CommitPop(std::move(lock));
        return true;
    }

    /** Pop message from queue. Blocks until the message is available or the
     * specified timeout expires.
     * @param msg Message object to assign.
     * @param timeout Timeout to wait message for.
     * @param lock Lock held if any. Should be previously acquired by @ref Lock()
     *      method.
     * @return true if message popped, false otherwise (e.g. timeout expired or
     *      exit requested).
     */
    template<class Rep, class Period>
    bool
    Pop(T &msg, const std::chrono::duration<Rep, Period>& timeout,
        QueueLock &&lock = QueueLock())
    {
        if (!lock) {
            lock = QueueLock(mutex);
        }
        lock = AcquirePop(std::move(lock), timeout);
        if (!lock) {
            return false;
        }
        msg = queue.front();
        queue.pop();
        CommitPop(std::move(lock));
        return true;
    }

    /** Wait until queue is empty. */
    void
    WaitEmpty(QueueLock &&lock = QueueLock())
    {
        if (!lock) {
            lock = QueueLock(mutex);
        }
        internal::MessageQueueBase::WaitEmpty(std::move(lock));
    }

    /** Wait until queue is empty, or timeout expires.
     * @return true if Wait succeeded, false if timeout expired.
     */
    template<class Rep, class Period>
    bool
    WaitEmpty(const std::chrono::duration<Rep, Period>& timeout,
              QueueLock &&lock = QueueLock())
    {
        if (!lock) {
            lock = QueueLock(mutex);
        }
        return internal::MessageQueueBase::WaitEmpty(std::move(lock), timeout);
    }

private:
    std::queue<T> queue;
};

} /* namespace adk */

#endif /* MESSAGE_QUEUE_H_ */
