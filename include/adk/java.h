/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2016, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See LICENSE file for copyright details.
 */

/** @file java.h
 * JNI support.
 */

#ifndef INCLUDE_ADK_JAVA_H_
#define INCLUDE_ADK_JAVA_H_

#include <jni.h>

namespace adk {

/** Current thread JNI environment if any attached. */
extern thread_local JNIEnv *jniEnv;

/* Java exceptions handling. */

/** Used to forward up pending Java exception raised by some JNI call.
 * Use JAVA_EXCEPTION_CHECK macro. Java exception is cleared on construction,
 * use Throw() method to raise it again.
 */
class JavaPendingException: public Exception {
public:
    JavaPendingException(const char *file, int line, jthrowable e);

    void
    Throw();
private:
    jthrowable e;
};

/** Throwing instance of this class causes associated Java exception to be
 * raised in the managed code.
 */
class JavaException: public Exception {
public:
    /** Throws Java RuntimeException. */
    JavaException(const char *file, int line, std::string msg);

    /** Throws exception with the specified class name. */
    JavaException(const char *file, int line, const char *excClassName,
                  std::string msg);

    /** Throws exception with the specified class. */
    JavaException(const char *file, int line, jclass excClass, std::string msg);

    /** Raise Java exception in the current Java environment. It does not throw
     * C++ exception so the function returns normally.
     */
    void
    Throw();

private:
    jclass excClass;
};

#define __ADK_DEF_JAVA_EXCEPTION(__clsName, __excName) \
    class __clsName: public JavaException { \
    public: \
        __clsName(const char *file, int line, std::string msg): \
            JavaException(file, line, "java/lang/" ADK_STR(__excName), msg) \
        {} \
    };

/* Wrappers for some common Java exceptions. */
__ADK_DEF_JAVA_EXCEPTION(OutOfMemoryException, OutOfMemoryError)
__ADK_DEF_JAVA_EXCEPTION(ArithmeticException, ArithmeticException)
__ADK_DEF_JAVA_EXCEPTION(IllegalArgumentException, IllegalArgumentException)
__ADK_DEF_JAVA_EXCEPTION(IllegalStateException, IllegalStateException)
__ADK_DEF_JAVA_EXCEPTION(IndexOutOfBoundsException, IndexOutOfBoundsException)
__ADK_DEF_JAVA_EXCEPTION(NullPointerException, NullPointerException)
__ADK_DEF_JAVA_EXCEPTION(UnsupportedOperationException, UnsupportedOperationException)

/** Check for pending Java exception after JNI call. Raise corresponding C++
 * exception if any.
 */
#define ADK_JAVA_EXCEPTION_CHECK() \
    do { \
        jthrowable e = jniEnv->ExceptionOccurred(); \
        if (e) { \
            throw JavaPendingException(__FILE__, __LINE__, e); \
        } \
    } while (false)

/** RAII-based Java exception checking. */
class JavaExceptionGuard {
public:
    ~JavaExceptionGuard()
    {
        ADK_JAVA_EXCEPTION_CHECK();
    }
};

// /////////////////////////////////////////////////////////////////////////////

/** Wrapper for references to Java objects. Handles automatic reference deletion
 * as well as global and local references
 */
template<class T>
class JavaRef {
public:
    /** Make local reference for the passed object.
     *
     * @param stealRef Steal the existing local reference of the passed object.
     *      For example, use it with the result of various Get* methods of JNI.
     */
    static JavaRef
    MakeLocal(T object, bool stealRef = true)
    {
        return JavaRef(object, false, stealRef);
    }

    static JavaRef
    MakeLocal(const JavaRef &ref)
    {
        if (ref) {
            if (ref.tag->isGlobal) {
                return JavaRef(ref.tag->object, false, false);
            } else {
                return JavaRef(ref);
            }
        } else {
            return JavaRef();
        }
    }

    /** Make global reference (perceived between native calls) for the passed
     * object.
     */
    static JavaRef
    MakeGlobal(T object)
    {
        return JavaRef(object, true, false);
    }

    static JavaRef
    MakeGlobal(const JavaRef &ref)
    {
        if (ref) {
            if (ref.tag->isGlobal) {
                return JavaRef(ref);
            } else {
                return JavaRef(ref.tag->object, true, false);
            }
        } else {
            return JavaRef();
        }
    }

    JavaRef() = default;

    JavaRef(const JavaRef &ref):
        tag(ref.tag)
    {}

    JavaRef(JavaRef &&ref):
        tag(std::move(ref.tag))
    {}

    JavaRef(T object, bool isGlobal = false, bool stealRef = true)
    {
        if (object) {
            tag = std::make_shared<Tag>(object, isGlobal, stealRef);
        }
    }

    template <typename U>
    JavaRef<U>
    Cast() const
    {
        return JavaRef<U>(reinterpret_cast<U>(**this), IsGlobal(), false);
    }

    /** Create new instance of Java object. Do not forget to call Pin() method
     * if it will be returned to JVM from JNI call.
     */
    template <typename... T_args>
    static JavaRef<T>
    Create(const std::string &clsName, const std::string &constrSignature,
           T_args... args)
    {
        JavaRef<jclass> cls =
            JavaRef<jclass>::MakeLocal(jniEnv->FindClass(clsName.c_str()));
        ADK_JAVA_EXCEPTION_CHECK();
        jmethodID methodID = jniEnv->GetMethodID(*cls, "<init>",
                                                 constrSignature.c_str());
        ADK_JAVA_EXCEPTION_CHECK();
        /* Catch exception in constructor. */
        JavaExceptionGuard excGuard;
        return MakeLocal(jniEnv->NewObject(*cls, methodID, args...));
    }

    /** Create new instance of Java object. Do not forget to call Pin() method
     * if it will be returned to JVM from JNI call.
     */
    static JavaRef<T>
    Create(const std::string &clsName)
    {
        return Create(clsName, "()V");
    }

    operator bool() const
    {
        return static_cast<bool>(tag);
    }

    T
    operator *() const
    {
        if (tag) {
            return tag->object;
        } else {
            return nullptr;
        }
    }

    /** Get additional local reference. */
    T
    GetLocal()
    {
        if (tag) {
            return reinterpret_cast<T>(
                jniEnv->NewLocalRef(reinterpret_cast<jobject>(tag->object)));
        } else {
            return nullptr;
        }
    }

    bool
    IsGlobal() const
    {
        if (tag) {
            return tag->isGlobal;
        } else {
            return false;
        }
    }

    JavaRef<T> &
    Pin()
    {
        if (tag) {
            tag->isPinned = true;
        }
        return *this;
    }

    JavaRef<T> &
    Unpin()
    {
        if (tag) {
            tag->isPinned = false;
        }
        return *this;
    }

    template <typename T_ret, typename... T_args>
    T_ret
    CallMethod(const std::string &methodName, const std::string &methodSignature,
               T_args... args);

    template <typename T_ret, typename... T_args>
    T_ret
    CallMethod(jmethodID mid, T_args... args);

    template <typename TField>
    TField
    GetField(const std::string &name);

    JavaRef<jobject>
    GetField(const std::string &name, const char *signature);

    template <typename TField>
    void
    SetField(const std::string &name, TField value);

    void
    SetField(const std::string &name, const char *signature, jobject value);

private:
    class Tag {
    public:
        T object;
        bool isGlobal, isPinned = false;

        Tag(T object, bool isGlobal, bool stealRef):
            isGlobal(isGlobal)
        {
            if (isGlobal) {
                this->object = reinterpret_cast<T>(
                    jniEnv->NewGlobalRef(reinterpret_cast<jobject>(object)));
            } else if (stealRef) {
                this->object = object;
            } else {
                this->object = reinterpret_cast<T>(
                    jniEnv->NewLocalRef(reinterpret_cast<jobject>(object)));
            }
        }

        ~Tag()
        {
            if (!isPinned) {
                if (isGlobal) {
                    jniEnv->DeleteGlobalRef(reinterpret_cast<jobject>(object));
                } else {
                    jniEnv->DeleteLocalRef(reinterpret_cast<jobject>(object));
                }
            }
        }
    };

    std::shared_ptr<Tag> tag;
};

// /////////////////////////////////////////////////////////////////////////////

template <typename T>
class BoxedValue;

namespace java_internals {

template <typename T_ret>
struct MethodCallSelector {};

template <>
struct MethodCallSelector<void> {
    template <typename... T_args>
    static void
    Call(jobject obj, jmethodID method_id, T_args... args)
    {
        jniEnv->CallVoidMethod(obj, method_id, args...);
        ADK_JAVA_EXCEPTION_CHECK();
    }
};

#define _ADK_JAVA_DEF_METHOD_SELECTOR(__type, __name, __cast) \
    template <> \
    struct MethodCallSelector<__type> { \
        template <typename... T_args> \
        static __type \
        Call(jobject obj, jmethodID methodId, T_args... args) \
        { \
            __type res = __cast(ADK_CONCAT( \
                ADK_CONCAT(jniEnv->Call,__name), Method)(obj, methodId, args...)); \
            ADK_JAVA_EXCEPTION_CHECK(); \
            return res; \
        } \
    };

#define __ADK_JAVA_DEF_NO_CAST

#define ADK_JAVA_DEF_METHOD_SELECTOR(__type, __name) \
    _ADK_JAVA_DEF_METHOD_SELECTOR(__type, __name, __ADK_JAVA_DEF_NO_CAST)

#define ADK_JAVA_DEF_METHOD_SELECTOR_CAST(__type, __name) \
    _ADK_JAVA_DEF_METHOD_SELECTOR(__type, __name, reinterpret_cast<__type>)

ADK_JAVA_DEF_METHOD_SELECTOR(jobject, Object)
ADK_JAVA_DEF_METHOD_SELECTOR_CAST(jstring, Object)

ADK_JAVA_DEF_METHOD_SELECTOR(jboolean, Boolean)
ADK_JAVA_DEF_METHOD_SELECTOR(jbyte, Byte)
ADK_JAVA_DEF_METHOD_SELECTOR(jchar, Char)
ADK_JAVA_DEF_METHOD_SELECTOR(jshort, Short)
ADK_JAVA_DEF_METHOD_SELECTOR(jint, Int)
ADK_JAVA_DEF_METHOD_SELECTOR(jlong, Long)
ADK_JAVA_DEF_METHOD_SELECTOR(jfloat, Float)
ADK_JAVA_DEF_METHOD_SELECTOR(jdouble, Double)

} /* namespace java_internals */

// /////////////////////////////////////////////////////////////////////////////
namespace java_internals {

template <typename T>
struct PrimitiveSignature {
    /*
    static const char *
    Get();

    static const char *
    GetBoxed();
     */
};

#define _ADK_JAVA_DEF_PRIMITIVE_SIG(__type, __name, __boxedName) \
    template <> \
    struct PrimitiveSignature<__type> { \
        static const char * \
        Get() \
        { \
            return ADK_STR(__name); \
        } \
        \
        static const char * \
        GetBoxed() \
        { \
            return "Ljava/lang/" ADK_STR(__boxedName) ";"; \
        } \
    };

_ADK_JAVA_DEF_PRIMITIVE_SIG(jboolean, Z, Boolean)
_ADK_JAVA_DEF_PRIMITIVE_SIG(jbyte, B, Byte)
_ADK_JAVA_DEF_PRIMITIVE_SIG(jchar, C, Char)
_ADK_JAVA_DEF_PRIMITIVE_SIG(jshort, S, Short)
_ADK_JAVA_DEF_PRIMITIVE_SIG(jint, I, Integer)
_ADK_JAVA_DEF_PRIMITIVE_SIG(jlong, J, Long)
_ADK_JAVA_DEF_PRIMITIVE_SIG(jfloat, F, Float)
_ADK_JAVA_DEF_PRIMITIVE_SIG(jdouble, D, Double)

template <typename T>
struct PrimitiveSignature<BoxedValue<T>> {
    static const char *
    Get()
    {
        return PrimitiveSignature<T>::GetBoxed();
    }

    static const char *
    GetBoxed()
    {
        return PrimitiveSignature<T>::GetBoxed();
    }
};

} /* namespace java_internals */

// /////////////////////////////////////////////////////////////////////////////

namespace java_internals {

//XXX set

template <typename T>
struct FieldAccessSelector {
    /*
    static T
    Get(jobject obj, jfieldID fieldId);

    static void
    Set(jobject obj, jfieldID fieldId, T value);
     */
};

#define _ADK_JAVA_DEF_FIELD_ACCESS_SELECTOR(__type, __name, __cast) \
    template <> \
    struct FieldAccessSelector<__type> { \
        static __type \
        Get(jobject obj, jfieldID fieldId) \
        { \
            __type res = __cast(ADK_CONCAT( \
                ADK_CONCAT(jniEnv->Get,__name), Field)(obj, fieldId)); \
            ADK_JAVA_EXCEPTION_CHECK(); \
            return res; \
        } \
        \
        static void \
        Set(jobject obj, jfieldID fieldId, __type value) \
        { \
            ADK_CONCAT(ADK_CONCAT(jniEnv->Set,__name), Field)(obj, fieldId, value); \
            ADK_JAVA_EXCEPTION_CHECK(); \
        } \
    };

#define ADK_JAVA_DEF_FIELD_ACCESS_SELECTOR(__type, __name) \
    _ADK_JAVA_DEF_FIELD_ACCESS_SELECTOR(__type, __name, __ADK_JAVA_DEF_NO_CAST)

#define ADK_JAVA_DEF_FIELD_ACCESS_SELECTOR_CAST(__type, __name) \
    _ADK_JAVA_DEF_FIELD_ACCESS_SELECTOR(__type, __name, reinterpret_cast<__type>)

ADK_JAVA_DEF_FIELD_ACCESS_SELECTOR(jobject, Object)
ADK_JAVA_DEF_FIELD_ACCESS_SELECTOR_CAST(jstring, Object)

ADK_JAVA_DEF_FIELD_ACCESS_SELECTOR(jboolean, Boolean)
ADK_JAVA_DEF_FIELD_ACCESS_SELECTOR(jbyte, Byte)
ADK_JAVA_DEF_FIELD_ACCESS_SELECTOR(jchar, Char)
ADK_JAVA_DEF_FIELD_ACCESS_SELECTOR(jshort, Short)
ADK_JAVA_DEF_FIELD_ACCESS_SELECTOR(jint, Int)
ADK_JAVA_DEF_FIELD_ACCESS_SELECTOR(jlong, Long)
ADK_JAVA_DEF_FIELD_ACCESS_SELECTOR(jfloat, Float)
ADK_JAVA_DEF_FIELD_ACCESS_SELECTOR(jdouble, Double)

template <typename T>
struct FieldAccessSelector<BoxedValue<T>> {
    static BoxedValue<T>
    Get(jobject obj, jfieldID fieldId)
    {
        jobject res = FieldAccessSelector<jobject>::Get(obj, fieldId);
        ADK_JAVA_EXCEPTION_CHECK();
        return res;
    }
};

} /* namespace java_internals */

// /////////////////////////////////////////////////////////////////////////////

namespace java_internals {

template <typename T>
struct PrimitiveArrayAccessor {
    /*
    static T *
    GetElements(jobject array, jboolean *isCopy);

    static void
    ReleaseElements(jobject array, T *elements, jint mode);
    */
};

#define ADK_JAVA_DEF_PRIMITIVE_ARRAY_ACCESSOR(__nativeType, __name) \
    template <> \
    struct PrimitiveArrayAccessor<__nativeType> { \
        static __nativeType *\
        GetElements(jarray array, jboolean *isCopy) \
        { \
            __nativeType *result = ADK_CONCAT(ADK_CONCAT(jniEnv->Get,__name), ArrayElements) \
                (reinterpret_cast<ADK_CONCAT(__nativeType, Array)>(array), isCopy); \
            ADK_JAVA_EXCEPTION_CHECK(); \
            return result; \
        } \
        \
        static void \
        ReleaseElements(jarray array, __nativeType *elements, jint mode) \
        { \
            ADK_CONCAT(ADK_CONCAT(jniEnv->Release,__name), ArrayElements) \
                (reinterpret_cast<ADK_CONCAT(__nativeType, Array)>(array), elements, mode); \
            ADK_JAVA_EXCEPTION_CHECK(); \
        } \
    };

ADK_JAVA_DEF_PRIMITIVE_ARRAY_ACCESSOR(jboolean, Boolean)
ADK_JAVA_DEF_PRIMITIVE_ARRAY_ACCESSOR(jbyte, Byte)
ADK_JAVA_DEF_PRIMITIVE_ARRAY_ACCESSOR(jchar, Char)
ADK_JAVA_DEF_PRIMITIVE_ARRAY_ACCESSOR(jshort, Short)
ADK_JAVA_DEF_PRIMITIVE_ARRAY_ACCESSOR(jint, Int)
ADK_JAVA_DEF_PRIMITIVE_ARRAY_ACCESSOR(jlong, Long)
ADK_JAVA_DEF_PRIMITIVE_ARRAY_ACCESSOR(jfloat, Float)
ADK_JAVA_DEF_PRIMITIVE_ARRAY_ACCESSOR(jdouble, Double)


class ArrayBase {
public:
    ArrayBase(jobject array):
        array(JavaRef<jarray>::MakeLocal(reinterpret_cast<jarray>(array)))
    {}

    ArrayBase(const JavaRef<jobject> &array):
        array(array.Cast<jarray>())
    {}

    size_t
    Size() const
    {
        return jniEnv->GetArrayLength(*array);
    }

    jarray
    GetObject() const
    {
        return *array;
    }

    operator bool() const
    {
        return array;
    }

protected:
    JavaRef<jarray> array;
};

template <typename T>
class PrimitiveArray: public ArrayBase {
public:
    PrimitiveArray(jobject array):
        ArrayBase(array)
    {
        elements = PrimitiveArrayAccessor<T>::GetElements(*this->array, nullptr);
    }

    PrimitiveArray(const JavaRef<jobject> &array):
        ArrayBase(array)
    {
        elements = PrimitiveArrayAccessor<T>::GetElements(*this->array, nullptr);
    }

    ~PrimitiveArray()
    {
        PrimitiveArrayAccessor<T>::ReleaseElements(*array, elements, 0);
    }

    T &
    Get(size_t idx)
    {
        return elements[idx];
    }

    T &
    operator[](size_t idx)
    {
        return Get(idx);
    }
private:
    T *elements;
};

class ObjectArray: public ArrayBase {
public:
    using ArrayBase::ArrayBase;

    JavaRef<jobject>
    Get(size_t idx)
    {
        jobject res = jniEnv->GetObjectArrayElement(
            reinterpret_cast<jobjectArray>(*array), idx);
        ADK_JAVA_EXCEPTION_CHECK();
        return JavaRef<jobject>::MakeLocal(res, false);
    }

    void
    Set(size_t idx, JavaRef<jobject> item)
    {
        jniEnv->SetObjectArrayElement(reinterpret_cast<jobjectArray>(*array),
                                      idx, *item);
        ADK_JAVA_EXCEPTION_CHECK();
    }

    JavaRef<jobject>
    operator[](size_t idx)
    {
        return Get(idx);
    }
};

template <typename T>
class Array: public PrimitiveArray<T> {
    using PrimitiveArray<T>::PrimitiveArray;
};

template <>
class Array<jobject>: public ObjectArray {
    using ObjectArray::ObjectArray;
};

} /* namespace java_internals */

// /////////////////////////////////////////////////////////////////////////////

namespace java_internals {

class BoxedValueBase {
public:
    BoxedValueBase() = default;

    BoxedValueBase(jobject obj):
        obj(obj)
    {}

    operator bool()
    {
        return static_cast<bool>(obj);
    }

protected:
    JavaRef<jobject> obj;
};

template <typename T>
struct BoxedValueHelper {
    /*
    static jobject
    Create(T value);

    static T
    GetValue(jobject obj);
     */
};

#define _ADK_JAVA_DEF_BOXED_VALUE_HELPER(__type, __name, __getterName) \
    template <> \
    struct BoxedValueHelper<__type> { \
        static jobject \
        Create(__type value) \
        { \
            JavaRef<jclass> cls = \
                JavaRef<jclass>::MakeLocal(jniEnv->FindClass("java/lang/" ADK_STR(__name))); \
            std::string sig("("); \
            sig += PrimitiveSignature<__type>::Get(); \
            sig += ")V"; \
            jmethodID methodID = jniEnv->GetMethodID(*cls, "<init>", sig.c_str()); \
            return jniEnv->NewObject(*cls, methodID, value); \
        } \
        \
        static __type \
        GetValue(JavaRef<jobject> obj) \
        { \
            std::string sig("()"); \
            sig += PrimitiveSignature<__type>::Get(); \
            return obj.CallMethod<__type>(ADK_STR(__getterName) "Value", sig.c_str()); \
        } \
    };

#define ADK_JAVA_DEF_BOXED_VALUE_HELPER(__type, __name) \
    _ADK_JAVA_DEF_BOXED_VALUE_HELPER(ADK_CONCAT(j, __type), __name, __type)

ADK_JAVA_DEF_BOXED_VALUE_HELPER(boolean, Boolean)
ADK_JAVA_DEF_BOXED_VALUE_HELPER(byte, Byte)
ADK_JAVA_DEF_BOXED_VALUE_HELPER(char, Char)
ADK_JAVA_DEF_BOXED_VALUE_HELPER(short, Short)
ADK_JAVA_DEF_BOXED_VALUE_HELPER(int, Integer)
ADK_JAVA_DEF_BOXED_VALUE_HELPER(long, Long)
ADK_JAVA_DEF_BOXED_VALUE_HELPER(float, Float)
ADK_JAVA_DEF_BOXED_VALUE_HELPER(double, Double)

} /* namespace java_internals */

// /////////////////////////////////////////////////////////////////////////////

class Java {
public:

    template <typename T>
    using Array = java_internals::Array<T>;

    template <typename T_ret, typename... T_args>
    static T_ret
    CallMethod(jobject obj, const std::string &methodName,
               const std::string &methodSignature, T_args... args)
    {
        ASSERT(obj);
        JNIEnv *env = GetEnv();
        auto cls = JavaRef<jclass>::MakeLocal(env->GetObjectClass(obj));
        ADK_JAVA_EXCEPTION_CHECK();
        jmethodID mid = env->GetMethodID(*cls, methodName.c_str(),
                                         methodSignature.c_str());
        ADK_JAVA_EXCEPTION_CHECK();
        return java_internals::MethodCallSelector<T_ret>::Call(obj, mid, args...);
    }

    template <typename T_ret, typename... T_args>
    static T_ret
    CallMethod(jobject obj, jmethodID mid, T_args... args)
    {
        ASSERT(obj);
        return java_internals::MethodCallSelector<T_ret>::Call(obj, mid, args...);
    }

    template <typename T_ret, typename... T_args>
    static T_ret
    CallIfaceMethod(const std::string &methodName,
                    const std::string &methodSignature, T_args... args)
    {
        return CallMethod<T_ret, T_args...>(javaIfaceObj, methodName,
                                            methodSignature, args...);
    }

    template <typename T>
    static T
    GetField(jobject obj, const std::string &name)
    {
        ASSERT(obj);
        JNIEnv *env = GetEnv();
        auto cls = JavaRef<jclass>::MakeLocal(env->GetObjectClass(obj));
        ADK_JAVA_EXCEPTION_CHECK();
        jfieldID fid = env->GetFieldID(*cls, name.c_str(),
                                       java_internals::PrimitiveSignature<T>::Get());
        ADK_JAVA_EXCEPTION_CHECK();
        return GetField<T>(obj, fid);
    }

    static JavaRef<jobject>
    GetField(jobject obj, const std::string &name, const char *signature)
    {
        ASSERT(obj);
        JNIEnv *env = GetEnv();
        auto cls = JavaRef<jclass>::MakeLocal(env->GetObjectClass(obj));
        ADK_JAVA_EXCEPTION_CHECK();
        jfieldID fid = env->GetFieldID(*cls, name.c_str(), signature);
        ADK_JAVA_EXCEPTION_CHECK();
        return GetField<jobject>(obj, fid);
    }

    template <typename T>
    static T
    GetField(jobject obj, jfieldID fid)
    {
        return java_internals::FieldAccessSelector<T>::Get(obj, fid);
    }

    template <typename T>
    static void
    SetField(jobject obj, const std::string &name, T value)
    {
        ASSERT(obj);
        JNIEnv *env = GetEnv();
        auto cls = JavaRef<jclass>::MakeLocal(env->GetObjectClass(obj));
        ADK_JAVA_EXCEPTION_CHECK();
        jfieldID fid = env->GetFieldID(*cls, name.c_str(),
                                       java_internals::PrimitiveSignature<T>::Get());
        ADK_JAVA_EXCEPTION_CHECK();
        SetField<T>(obj, fid, value);
    }

    static void
    SetField(jobject obj, const std::string &name, const char *signature, jobject value)
    {
        ASSERT(obj);
        JNIEnv *env = GetEnv();
        auto cls = JavaRef<jclass>::MakeLocal(env->GetObjectClass(obj));
        ADK_JAVA_EXCEPTION_CHECK();
        jfieldID fid = env->GetFieldID(*cls, name.c_str(), signature);
        ADK_JAVA_EXCEPTION_CHECK();
        SetField<jobject>(obj, fid, value);
    }

    template <typename T>
    static void
    SetField(jobject obj, jfieldID fid, T value)
    {
        java_internals::FieldAccessSelector<T>::Set(obj, fid, value);
    }

    template <typename T>
    static Array<T>
    GetArray(jobject array)
    {
        return Array<T>(array);
    }

    static std::string
    GetString(jstring s)
    {
        ASSERT(s);
        jboolean isCopy;
        const char *chars = GetEnv()->GetStringUTFChars(s, &isCopy);
        ADK_JAVA_EXCEPTION_CHECK();
        if (!chars) {
            ADK_EXCEPTION(InternalErrorException, "Failed to get Java string data");
        }
        std::string res(chars);
        jniEnv->ReleaseStringUTFChars(reinterpret_cast<jstring>(s), chars);
        return res;
    }

    static JavaRef<jstring>
    WrapString(const std::string &s)
    {
        return GetEnv()->NewStringUTF(s.c_str());
    }

    static void
    Initialize(JNIEnv *env, jobject ifaceObj);

    /** Must be called before attached to VM thread is exiting. */
    static void
    DetachCurrentThread();

    static JNIEnv *
    GetEnv(JNIEnv *env = nullptr);

private:
    /** Java VM instance. */
    static JavaVM *javaVm;
    /** VSM instance on Java side. */
    static jobject javaIfaceObj;
};

// /////////////////////////////////////////////////////////////////////////////


template <typename T>
template <typename T_ret, typename... T_args>
T_ret
JavaRef<T>::CallMethod(const std::string &methodName,
                       const std::string &methodSignature, T_args... args)
{
    ASSERT(tag);
    return Java::CallMethod<T_ret, T_args...>(**this, methodName,
                                              methodSignature, args...);
}

template <typename T>
template <typename T_ret, typename... T_args>
T_ret
JavaRef<T>::CallMethod(jmethodID mid, T_args... args)
{
    ASSERT(tag);
    return Java::CallMethod<T_ret, T_args...>(**this, mid, args...);
}

template <typename T>
template <typename TField>
TField
JavaRef<T>::GetField(const std::string &name)
{
    ASSERT(tag);
    return Java::GetField<TField>(**this, name);
}

template <typename T>
JavaRef<jobject>
JavaRef<T>::GetField(const std::string &name, const char *signature)
{
    ASSERT(tag);
    return Java::GetField(**this, name, signature);
}

template <typename T>
template <typename TField>
void
JavaRef<T>::SetField(const std::string &name, TField value)
{
    ASSERT(tag);
    Java::SetField<TField>(**this, name, value);
}

template <typename T>
void
JavaRef<T>::SetField(const std::string &name, const char *signature,
                     jobject value)
{
    ASSERT(tag);
    Java::SetField(**this, name, signature, value);
}

// /////////////////////////////////////////////////////////////////////////////


template <typename T>
class BoxedValue: public java_internals::BoxedValueBase {
public:
    using BoxedValueBase::BoxedValueBase;

    BoxedValue(T value):
        BoxedValueBase(java_internals::BoxedValueHelper<T>::Create(value))
    {}

    T
    GetValue() const
    {
        ASSERT(obj);
        return java_internals::BoxedValueHelper<T>::GetValue(*obj);
    }

    T
    operator *() const
    {
        return GetValue();
    }
};

// /////////////////////////////////////////////////////////////////////////////


/** Wrapper for JNI method call. */
#define ADK_JNI_CALL(__method, ...) \
    ({ \
        auto __result = adk::jniEnv->__method(__VA_ARGS__); \
        ADK_JAVA_EXCEPTION_CHECK(); \
        __result; \
    })

/** All JNI exported methods should be wrapped into this macro. */
#define ADK_JNI_METHOD_START(__jniEnv) \
    { \
        adk::jniEnv = __jniEnv; \
        try

/** All JNI exported methods should be wrapped into this macro. */
#define ADK_JNI_METHOD_END(__resultType) \
        catch (adk::JavaException &e) { \
            e.Throw(); \
        } catch (adk::JavaPendingException &e) { \
            e.Throw(); \
        } catch (std::exception &e) { \
            adk::JavaException(__FILE__, __LINE__, e.what()).Throw(); \
        } \
        return __resultType(); \
    }

} /* namespace adk */

/* Auto-generated by javah native methods prototypes. */
#include <auto_adk_jni.h>

#endif /* INCLUDE_ADK_JAVA_H_ */
