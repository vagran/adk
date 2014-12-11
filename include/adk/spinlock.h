/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file spilock.h
 * Implementation of simple spin lock.
 */

#ifndef SPINLOCK_H_
#define SPINLOCK_H_

namespace adk {

/** Simple spin-lock. */
class Spinlock {
public:
    Spinlock() = default;
    Spinlock(const Spinlock &) = delete;
    Spinlock(Spinlock &&) = delete;

    void
    Lock()
    {
        while (lock.test_and_set(std::memory_order_acquire));
    }

    void
    Unlock()
    {
        lock.clear(std::memory_order_release);
    }

private:
    std::atomic_flag lock = ATOMIC_FLAG_INIT;
};

/** Guard object for safe locking of simple spin-lock. */
class SpinlockGuard {
public:
    SpinlockGuard():
        lock(nullptr), isLocked(false)
    {}

    SpinlockGuard(const SpinlockGuard &) = delete;

    SpinlockGuard(SpinlockGuard &&lg):
        lock(lg.lock), isLocked(lg.isLocked)
    {
        lg.lock = nullptr;
        lg.isLocked = false;
    }

    SpinlockGuard(Spinlock &lock, bool doLock = true):
        lock(&lock), isLocked(doLock)
    {
        if (doLock) {
            lock.Lock();
        }
    }

    ~SpinlockGuard()
    {
        if (lock && isLocked) {
            lock->Unlock();
        }
    }

    void
    operator =(SpinlockGuard &&lg)
    {
        if (lock && isLocked) {
            lock->Unlock();
        }
        lock = lg.lock;
        isLocked = lg.isLocked;
        lg.lock = nullptr;
        lg.isLocked = false;
    }

    void
    Lock()
    {
        ASSERT(lock);
        ASSERT(!isLocked);
        lock->Lock();
        isLocked = true;
    }

    void
    Unlock()
    {
        ASSERT(lock);
        ASSERT(isLocked);
        isLocked = false;
        lock->Unlock();
    }

    void
    Release()
    {
        ASSERT(lock);
        lock = nullptr;
        isLocked = false;
    }

private:
    Spinlock *lock;
    bool isLocked;
};

}

#endif /* SPINLOCK_H_ */
