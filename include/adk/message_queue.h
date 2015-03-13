/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#ifndef MESSAGE_QUEUE_H_
#define MESSAGE_QUEUE_H_

namespace adk {

namespace internal {

class MessageQueueBase {
public:
    typedef std::unique_lock<std::mutex> QueueLock;

    /** Acquire queue lock. Can be passed to queue methods. */
    QueueLock Lock();

    /** Request exit. */
    void
    Exit(QueueLock &&lock = QueueLock());

    /** Check if exit was requested. */
    bool
    IsExitRequested(QueueLock &&lock = QueueLock());

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
    MessageQueue(size_t maxLen = 0)
    {}

    /** Push message into the queue. Blocks if queue size limit reached.
     * @param msg Message to push.
     * @param lock Lock held if any. Should be previously acquired by @ref Lock()
     *      method.
     */
    void
    Push(const T &msg, QueueLock &&lock = QueueLock())
    {
        //XXX
    }

    /** Push message into the queue. Blocks if queue size limit reached.
     * @param msg Message to push.
     * @param lock Lock held if any. Should be previously acquired by @ref Lock()
     *      method.
     */
    void
    Push(T &&msg, QueueLock &&lock = QueueLock())
    {
        //XXX
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
        //XXX
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
        //XXX
    }
};

} /* namespace adk */

#endif /* MESSAGE_QUEUE_H_ */
