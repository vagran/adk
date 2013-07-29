/* /ADK/include/adk/signal.h
 *
 * This file is a part of 'ADK' project.
 * Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
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
        return std::forward<SlotTargetType>(slotTarget);
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
GetSlotTarget(Args &&... args)
{
    return GetSlotTargetImpl<Args...>::Get(std::forward<Args>(args)...);
}


/** Base class for slot. */
class SlotBase {
public:

    /** Get target object.
     *
     * @return Pointer to target object in case the slot was created from
     *      method of class derived from SlotTarget class. Otherwise nullptr
     *      is returned.
     */
    SlotTarget *
    GetTarget() const
    {
        return _target;
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
    Make(Args &&... args)
    {
        SlotTarget *slotTarget = adk_internal::GetSlotTarget(std::forward<Args>(args)...);
        ADK_INFO("st %p", slotTarget);//XXX
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
    operator ()(Args &&... args)
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

    ~SignalBaseSpec()
    {}

private:
    std::list<SlotType *> _slots;
};

} /* namespace adk_internal */

/** Signal object. Multiple slots with the same signature can be connected to
 * the signal object.
 */
template <typename Signature, typename = void>
class Signal: public adk_internal::SignalBaseSpec<Signature> {
public:
    typedef adk_internal::SignalBaseSpec<Signature> BaseType;

    template <typename... Args>
    typename BaseType::ResultType
    Emit(Args &&... args)
    {
        //XXX
        return BaseType::ResultType();
    }
};

template <typename Signature>
class Signal<Signature,
             typename std::enable_if<adk_internal::SignatureResultIsVoid<Signature>()>::type>:
    public adk_internal::SignalBaseSpec<Signature> {
public:
    template <typename... Args>
    void
    Emit(Args &&... args)
    {
        //XXX
    }
};

} /* namespace adk */

#endif /* SIGNAL_H_ */
