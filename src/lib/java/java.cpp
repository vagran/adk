/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2016, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See LICENSE file for copyright details.
 */

/** @file java.cpp
 * JNI support.
 */

#include <adk.h>

using namespace adk;

JavaVM *Java::javaVm;
jobject Java::javaIfaceObj;
thread_local JNIEnv *adk::jniEnv = nullptr;

void
Java::Initialize(JNIEnv *env, jobject ifaceObj)
{
    GetEnv(env);
    javaIfaceObj = env->NewGlobalRef(ifaceObj);
    if (env->GetJavaVM(&javaVm) != JNI_OK) {
        ADK_ERROR("Failed to get Java VM reference");
    }
}

void
Java::DetachCurrentThread()
{
    if (jniEnv) {
        javaVm->DetachCurrentThread();
        jniEnv = nullptr;
        ADK_DEBUG("Detached Java JNI environment from thread %zu",
                  std::hash<std::thread::id>()(std::this_thread::get_id()));
    }
}

JNIEnv *
Java::GetEnv(JNIEnv *env)
{
    if (!env) {
        if (!jniEnv) {
            if (javaVm->AttachCurrentThread(
                reinterpret_cast<void **>(&jniEnv), nullptr) != JNI_OK) {

                ADK_EXCEPTION(InternalErrorException, "Failed to attach thread to JVM");
            }
            ADK_DEBUG("Attached Java JNI environment to thread %zu",
                      std::hash<std::thread::id>()(std::this_thread::get_id()));
        }
        env = jniEnv;
    } else {
        jniEnv = env;
    }
    return env;
}

JavaPendingException::JavaPendingException(const char *file, int line, jthrowable e):
    Exception(file, line, std::string("Java pending exception detected: ")),
    e(e)
{
    jniEnv->ExceptionClear();
    _msg += Java::GetString(
        Java::CallMethod<jstring>(e, "toString", "()Ljava/lang/String;"));
}

void
JavaPendingException::Throw()
{
    if (jniEnv->Throw(e)) {
        printf("Throw() failed for exception: %s\n", _msg.c_str());
    }
}

JavaException::JavaException(const char *file, int line, jclass excClass,
                             std::string msg):
    Exception(file, line, msg), excClass(excClass)
{
    ADK_WARNING("Exception in native code: %s", this->_msg.c_str());
}

JavaException::JavaException(const char *file, int line, std::string msg):
    JavaException(file, line, "com/ast/utils/NativeComponent$NativeException", msg)
{}

JavaException::JavaException(const char *file, int line,
                             const char *excClassName, std::string msg):
    JavaException(file, line, ADK_JNI_CALL(FindClass, excClassName), msg)
{}

void
JavaException::Throw()
{
    if (jniEnv->ThrowNew(excClass, _msg.c_str())) {
        printf("ThrowNew() failed for exception: %s\n", _msg.c_str());
    }
}
