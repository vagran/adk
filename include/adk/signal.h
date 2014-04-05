/* /ADK/include/adk/signal.h
 *
 * This file is a part of 'ADK' project.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file signal.h
 * Signals and slots.
 *
 *
 */

#ifndef SIGNAL_H_
#define SIGNAL_H_

namespace adk {

namespace adk_internal {

class SlotBase;
class SignalBase;

} /* namespace adk_internal */

/** Base class for objects which have methods bound to slots and want slot
 * disconnection when the object is destroyed.
 */
class SlotTarget {
public:
    SlotTarget() = default;

    SlotTarget(const SlotTarget &)
    {
        /* Do not copy associated slots. */
    }

    SlotTarget(SlotTarget &&) = default;

    ~SlotTarget();

private:
    friend class adk_internal::SlotBase;

    std::list<adk_internal::SlotBase *> _slots;
    std::mutex _slotsMutex;

    void
    _RegisterSlot(adk_internal::SlotBase *slot);

    void
    _ReplaceSlot(adk_internal::SlotBase *slot, adk_internal::SlotBase *newSlot);

    void
    _RemoveSlot(adk_internal::SlotBase *slot);
};

namespace adk_internal {


/** Helper for retrieving result type from callable signature. */
template <typename Signature>
struct SignatureResultImpl {};

template <typename Result, typename... Args>
struct SignatureResultImpl<Result(Args...)> {
    typedef Result type;
};

template <typename Signature>
using SignatureResult = typename SignatureResultImpl<Signature>::type;


/** Check if the signature result is void. */
template <typename Signature>
constexpr bool
SignatureResultIsVoid()
{
    return std::is_same<SignatureResult<Signature>, void>::value;
}


template <typename Callable, typename = void>
struct IsSlotTargetMethodImpl {
    static constexpr bool value = false;
};

template <class Result, class Class, typename... Args>
struct IsSlotTargetMethodImpl<Result (Class::*)(Args...),
    typename std::enable_if<std::is_base_of<SlotTarget, Class>::value>::type> {

    static constexpr bool value = true;
};

/** Check if the callable object is SlotTarget class method. */
template <typename Callable>
constexpr bool
IsSlotTargetMethod()
{
    return IsSlotTargetMethodImpl<Callable>::value;
}


template <typename Method, class SlotTargetType, typename = void>
struct SlotTargetGetter {

    static constexpr SlotTarget *
    Get(SlotTargetType &&)
    {
        return nullptr;
    }
};

template <typename Method, class SlotTargetType>
struct SlotTargetGetter<Method, SlotTargetType,
                        typename std::enable_if<IsSlotTargetMethod<Method>()>::type> {

    static constexpr SlotTarget *
    Get(SlotTargetType &&slotTarget)
    {
        return slotTarget;
    }
};


template <typename... Args>
struct GetSlotTargetImpl {

    static constexpr SlotTarget *
    Get(Args &&...)
    {
        return nullptr;
    }
};

template <typename Method, class SlotTargetType, typename... Args>
struct GetSlotTargetImpl<Method, SlotTargetType, Args...> {

    static constexpr SlotTarget *
    Get(Method &&, SlotTargetType &&slotTarget, Args &&...)
    {
        return SlotTargetGetter<Method, SlotTargetType>::Get
            (std::forward<SlotTargetType>(slotTarget));
    }
};

/** Check if the first argument (callable) is SlotTarget method and return
 * object pointer if it is. Returns nullptr otherwise.
 */
template <typename... Args>
constexpr SlotTarget *
GetSlotTarget(Args &&...args)
{
    return GetSlotTargetImpl<Args...>::Get(std::forward<Args>(args)...);
}

/** Helper class for casting to classes derived from SlotTarget. */
template <class TargetType>
struct SlotTargetCast {
    static TargetType *
    Cast(SlotTarget *target)
    {
        return dynamic_cast<TargetType *>(target);
    }
};

template <>
struct SlotTargetCast<SlotTarget *> {
    static SlotTarget *
    Cast(SlotTarget *target)
    {
        return target;
    }
};

/** Base class for slot. */
class SlotBase {
public:

    /** Get target object.
     *
     * @return Pointer to target object in case the slot was created from
     *      method of class derived from SlotTarget class. Otherwise nullptr
     *      is returned.
     */
    template <class TargetType = SlotTarget>
    TargetType *
    GetTarget() const
    {
        return SlotTargetCast<TargetType>::Cast(_target);
    }

protected:
    friend class adk::SlotTarget;

    /** Target object if any. */
    SlotTarget *_target = nullptr;

    SlotBase() = default;

    SlotBase(SlotTarget *target):
        _target(target)
    {
        if (_target) {
            _target->_RegisterSlot(this);
        }
    }

    SlotBase(const SlotBase &slot):
        _target(slot._target)
    {
        if (_target) {
            _target->_RegisterSlot(this);
        }
    }

    SlotBase(SlotBase &&slot):
        _target(slot._target)
    {
        if (_target) {
            _target->_ReplaceSlot(&slot, this);
            slot._target = nullptr;
        }
    }

    ~SlotBase()
    {
        if (_target) {
            _target->_RemoveSlot(this);
        }
    }

    SlotBase &
    operator =(const SlotBase &slot)
    {
        if (slot._target == _target) {
            return *this;
        }
        if (_target) {
            _target->_RemoveSlot(this);
        }
        if (slot._target) {
            slot._target->_RegisterSlot(this);
        }
        _target = slot._target;
        return *this;
    }

    SlotBase &
    operator =(SlotBase &&slot)
    {
        if (_target && slot._target == _target) {
            _target->_RemoveSlot(&slot);
            slot._target = nullptr;
            return *this;
        }
        if (_target) {
            _target->_RemoveSlot(this);
        }
        if (slot._target) {
            slot._target->_ReplaceSlot(&slot, this);
        }
        _target = slot._target;
        slot._target = nullptr;
        return *this;
    }

    virtual void
    _Unbind() = 0;
};


/** Base class for signal. */
class SignalBase {
private:
    friend SlotBase;
};

} /* namespace adk_internal */

/** Slot which can be bound to any callable object.
 * @param Signature Slot invocation signature. User callable object should
 *      have compatible return type. User callable object can access passed
 *      arguments via std::placeholders members.
 */
template <typename Signature>
class Slot: public adk_internal::SlotBase {
public:
    Slot() = default;

    /** Underlying function type. */
    typedef std::function<Signature> FunctionType;
    /** Invocation result type. */
    typedef adk_internal::SignatureResult<Signature> ResultType;

    /** Create slot from any callable object. std::placeholders members can be
     * used to accept slot invocation arguments.
     */
    template <typename... Args>
    static Slot
    Make(Args &&...args)
    {
        SlotTarget *slotTarget = adk_internal::GetSlotTarget(std::forward<Args>(args)...);
        return Slot(std::bind(std::forward<Args>(args)...), slotTarget);
    }

    /** Check if slot is bound. */
    operator bool() const
    {
        return static_cast<bool>(_func);
    }

    /** Invoke the slot. */
    template <typename... Args>
    ResultType
    operator ()(Args &&...args)
    {
        return _func(std::forward<Args>(args)...);
    }

private:
    std::function<Signature> _func;

    /** @param func Created function.
     *  @param target Target object if any.
     */
    template <typename Func>
    Slot(Func &&func, SlotTarget *target):
        SlotBase(target),
        _func(std::forward<Func>(func))
    {}

    virtual void
    _Unbind() override
    {
        //XXX
        _func = nullptr;
        _target = nullptr;
    }
};


namespace adk_internal {

template <typename Signature>
class SignalBaseSpec: public adk_internal::SignalBase {
public:
    /** Corresponding slot type. */
    typedef Slot<Signature> SlotType;
    /** Emission result type. */
    typedef typename SlotType::ResultType ResultType;

private:
    friend class Connection;

    class SlotEntry {
    public:
        SlotType slot;
        /** Nullptr when removed. */
        SignalBaseSpec *signal;
        std::mutex mutex;

        SlotEntry(SignalBaseSpec *signal, SlotType slot):
            slot(slot), signal(signal)
        {}
    };

    typedef std::list<std::shared_ptr<SlotEntry>> SlotList;
    typedef typename SlotList::iterator SlotListIterator;

public:
    class Connection {
    public:
        void
        Disconnect()
        {
            std::shared_ptr<SlotEntry> e = this->e.lock();
            if (!e) {
                return;
            }
            std::unique_lock<std::mutex> lock(e->mutex);
            if (!e->signal) {
                return;class C {
                    int x = 0;

                    int
                    Method()
                    {
                        x = this->x * 2;
                        return x;
                    }
                };
            }
            e->signal->_Disconnect(e);
        }

        /** Check whether the connection is still valid. */
        operator bool()
        {
            std::shared_ptr<SlotEntry> e = this->e.lock();
            if (!e) {
                return false;
            }
            std::unique_lock<std::mutex> lock(e->mutex);
            return e->signal;
        }

    private:
        friend class SignalBaseSpec;

        std::weak_ptr<SlotEntry> e;

        /** Should be called with list lock acquired. */
        Connection(const std::shared_ptr<SlotEntry> &e):
            e(e)
        {}
    };

    Connection
    Connect(SlotType slot)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _slots.emplace_front(std::make_shared<SlotEntry>(this, slot));
        return Connection(_slots.front());
    }

    ~SignalBaseSpec()
    {
        /* Entry lock should be acquired before list lock so use this algorithm. */
        while (true) {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_slots.empty()) {
                break;
            }
            auto e = _slots.front();
            lock.unlock();
            std::unique_lock<std::mutex> entryLock(e->mutex);
            if (!e->signal) {
                /* Already removed. */
                continue;
            }
            lock.lock();
            _slots.remove(e);
            e->signal = nullptr;
        }
    }

protected:
    /** Get list of slots for signal emission. */
    std::list<SlotType>
    _GetEmitSlots()
    {
        std::list<std::shared_ptr<SlotEntry>> entries;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            for (const std::shared_ptr<SlotEntry> &e: _slots) {
                entries.emplace_back(e);
            }
        }
        std::list<SlotType> slots;
        for (const std::shared_ptr<SlotEntry> &e: entries) {
            std::unique_lock<std::mutex> lock(e->mutex);
            if (!e->signal) {
                continue;
            }
            if (!e->slot) {
                std::unique_lock<std::mutex> listLock(_mutex);
                _slots.remove(e);
                e->signal = nullptr;
                continue;
            }
            slots.emplace_back(e->slot);
        }
        return slots;
    }

private:
    SlotList _slots;
    std::mutex _mutex;

    /** For use from connection object. Entry object should be locked. */
    void
    _Disconnect(std::shared_ptr<SlotEntry> e)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _slots.remove(e);
        e->signal = nullptr;
    }
};

} /* namespace adk_internal */

/** Signal object. Multiple slots with the same signature can be connected to
 * the signal object.
 */
template <typename Signature, typename = void>
class Signal: public adk_internal::SignalBaseSpec<Signature> {
public:
    typedef adk_internal::SignalBaseSpec<Signature> BaseType;

    /** Default result mapper.
     * Result mapper can be used to map slots returned values to some final
     * result value. It also can stop slots invocation if necessary.
     * Default implementation never stops slot invocations and returns result
     * from the last slot invocation.
     */
    class DefResultMapper {
    public:
        DefResultMapper():
            _lastResult()
        {}

        /** This method should return true to continue slots invocations, and
         * false to stop them.
         * @param result Result from last invocation.
         */
        bool
        ProcessResult(typename BaseType::ResultType &&result)
        {
            _lastResult = result;
            return true;
        }

        /** Get final result to return from Emit() method. */
        typename BaseType::ResultType
        GetResult()
        {
            return _lastResult;
        }

    private:
        typename BaseType::ResultType _lastResult;
    };

    /** Result mapper for ignoring result value. */
    class VoidResultMapper {
    public:
        bool
        ProcessResult(typename BaseType::ResultType &&)
        {
            return true;
        }

        void
        GetResult()
        {}
    };

    /** Emit the signal and map result values to the final result.
     *
     * @param resultMapper Mapper object.
     * @param args Arguments for connected slots.
     * @return Final result from mapper object.
     */
    template <typename ResultMapper, typename... Args>
    decltype(std::declval<ResultMapper>().GetResult())
    EmitMap(ResultMapper &&resultMapper, Args &&...args)
    {
        auto slots = BaseType::_GetEmitSlots();
        for (auto &slot: slots) {
            if (!resultMapper.ProcessResult(slot(args...))) {
                break;
            }
        }
        return resultMapper.GetResult();
    }

    /** Emit the signal.
     *
     * @param args Arguments for connected slots.
     * @return Last slot result.
     */
    template <typename... Args>
    typename BaseType::ResultType
    Emit(Args &&...args)
    {
        return EmitMap(DefResultMapper(), std::forward<Args>(args)...);
    }

    /** Emit the signal and ignore results from slots. */
    template <typename... Args>
    void
    EmitNoResult(Args &&...args)
    {
        return EmitMap(VoidResultMapper(), std::forward<Args>(args)...);
    }
};

template <typename Signature>
class Signal<Signature,
             typename std::enable_if<adk_internal::SignatureResultIsVoid<Signature>()>::type>:
    public adk_internal::SignalBaseSpec<Signature> {
public:
    typedef adk_internal::SignalBaseSpec<Signature> BaseType;

    template <typename... Args>
    void
    Emit(Args ... args)
    {
        auto slots = BaseType::_GetEmitSlots();
        for (auto &slot: slots) {
            slot(args...);
        }
    }
};

/** Signal connection represents connected slot. */
template <typename Signature>
using SignalConnection = typename adk_internal::SignalBaseSpec<Signature>::Connection;

} /* namespace adk */

#endif /* SIGNAL_H_ */
