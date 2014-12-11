/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file optional.h
 * Implementation for Optional class.
 */

#ifndef OPTIONAL_H_
#define OPTIONAL_H_

namespace adk {

/** Type for null option. Used to disengage optional value. */
struct Nullopt_t {};

/** Null option. Used to disengage optional value. */
constexpr Nullopt_t nullopt;

/** The class encapsulates optional value of type T. */
template <typename T>
class Optional {
public:

    constexpr Optional():
        isValid(false)
    {}

    constexpr Optional(Nullopt_t):
        isValid(false)
    {}

    Optional(const T &value):
        isValid(true)
    {
        new(storage.data) T(value);
    }

    Optional(T &&value):
        isValid(true)
    {
        new(storage.data) T(std::move(value));
    }

    Optional(const Optional &value):
        isValid(value.isValid)
    {
        if (isValid) {
            new(storage.data) T(*value);
        }
    }

    Optional(Optional &&value):
        isValid(value.isValid)
    {
        if (isValid) {
            new(storage.data) T(std::move(*value));
        }
    }

    ~Optional()
    {
        Disengage();
    }

    Optional &
    operator =(Nullopt_t)
    {
        Disengage();
        return *this;
    }

    Optional &
    operator =(const T &value)
    {
        if (isValid) {
            **this = value;
        } else {
            new(storage.data) T(value);
            isValid = true;
        }
        return *this;
    }

    Optional &
    operator =(T &&value)
    {
        if (isValid) {
            **this = std::move(value);
        } else {
            new(storage.data) T(std::move(value));
            isValid = true;
        }
        return *this;
    }

    Optional &
    operator =(const Optional &value)
    {
        if (isValid) {
            if (value.isValid) {
                **this = *value;
            } else {
                (**this).~T();
                isValid = false;
            }
        } else if (value.isValid) {
            new(storage.data) T(*value);
            isValid = true;
        }
        return *this;
    }

    Optional &
    operator =(Optional &&value)
    {
        if (isValid) {
            if (value.isValid) {
                **this = *std::move(value);
            } else {
                (**this).~T();
                isValid = false;
            }
        } else if (value.isValid) {
            new(storage.data) T(*std::move(value));
            isValid = true;
        }
        return *this;
    }

    template <typename U,
              typename = typename std::enable_if<std::is_constructible<T, U>::value>::type,
              typename = typename std::enable_if<std::is_assignable<T, U>::value>::type>
    Optional &
    operator =(U &&value)
    {
        if (isValid) {
            (**this) = std::forward<U>(value);
        } else {
            new(storage.data) T(std::forward<U>(value));
            isValid = true;
        }
        return *this;
    }

    T &
    operator *() &
    {
        ASSERT(isValid);
        return *GetPtr();
    }

#   ifndef DEBUG
    constexpr
#   endif
    const T &
    operator *() const &
    {
        ASSERT(isValid);
        return *GetPtr();
    }

    T &&
    operator *() &&
    {
        ASSERT(isValid);
        return std::move(*GetPtr());
    }

    T *
    operator ->() &
    {
        return &(**this);
    }

#   ifndef DEBUG
    constexpr
#   endif
    const T *
    operator ->() const &
    {
        return &(**this);
    }

    constexpr explicit operator bool() const
    {
        return isValid;
    }

    void
    Disengage()
    {
        if (isValid) {
            (**this).~T();
            isValid = false;
#           ifdef DEBUG
            memset(storage.data, 0xfe, sizeof(storage.data));
#           endif
        }
    }

private:
    /** Storage for the value. */
    union {
        u8 data[sizeof(T)];
        /** Ensure the data array is always aligned as long. */
        long align;
    } storage;
    bool isValid;

    inline T *
    GetPtr()
    {
        return reinterpret_cast<T *>(storage.data);
    }

    inline const T *
    GetPtr() const
    {
        return reinterpret_cast<const T *>(storage.data);
    }
};

} /* namespace adk */

#endif /* OPTIONAL_H_ */
