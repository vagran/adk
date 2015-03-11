/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file static_init.h
 * Static initialization stuff.
 */

#ifndef STATIC_INIT_H_
#define STATIC_INIT_H_

namespace adk {

/** Helper class for static initialization based on Schwarz Counter. */
template <class T>
class StaticInitializer {
private:
    T **ppObj;
    int &counter;
public:
    typedef T ObjType;

    template <typename... TArgs>
    StaticInitializer(int &counter, T** ppObj, TArgs &&... args):
        ppObj(ppObj), counter(counter)
    {
        if (0 == counter++) {
            *ppObj = new T(std::forward<TArgs>(args)...);
        }
    }

    ~StaticInitializer()
    {
        if (0 == --counter) {
            delete *ppObj;
        }
    }
};

/** Declare static object initialization. Should be placed in the header after
 * the object pointer declaration. This ensures the initialization will be done
 * before any usage of the object from other static objects in the same
 * compilation unit.
 * @param __name Name for initializer class.
 * @param __objPtr Object pointer variable.
 * @param __VA_ARSG__ parameters for the object constructor.
 */
#define ADK_STATIC_INIT_DECL(__name, __objPtr, ...) \
    class __name: public adk::StaticInitializer<typename std::remove_reference<decltype(*(__objPtr))>::type> { \
    private: \
        static int __counter; \
    public: \
        template <typename... TArgs> \
        __name(ObjType **__ppObj, TArgs &&... args): \
            StaticInitializer(__counter, __ppObj, std::forward<TArgs>(args)...) \
        {} \
    }; \
    namespace { \
        __name __UID(static_init_) {&(__objPtr), ## __VA_ARGS__ }; \
    }

/** Should be placed in some compilation unit.
 * @param __name Name used in ADK_STATIC_INIT_DECL.
 */
#define ADK_STATIC_INIT_DEF(__name) \
    int __name::__counter;

} /* namespace adk */

#endif /* STATIC_INIT_H_ */
