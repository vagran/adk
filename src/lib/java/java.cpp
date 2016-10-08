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

namespace {

class JavaLogger {
public:
    static void
    Initialize(const std::string &level)
    {
        instance = std::unique_ptr<JavaLogger>(new JavaLogger(LevelFromString(level)));
        instance->oldLogFunc = Log::GetLogFunc();
        Log::SetLogFunc(WriteV);
    }

    static void
    Shutdown()
    {
        Log::SetLogFunc(instance->oldLogFunc);
        instance = nullptr;
    }

    static void
    WriteV(Log::Level level, const char *msg, va_list args)
    {
        if (level > instance->thresholdLevel) {
            return;
        }
        char buf[2048];
        size_t size = vsnprintf(buf, sizeof(buf), msg, args);
        if (static_cast<ssize_t>(size) < 0) {
            /* Formatting error */
            return;
        }
        auto levelStr = Java::WrapString(LevelToString(level));
        auto msgStr = Java::WrapString(std::string(buf, std::min(sizeof(buf), size)));
        Java::CallIfaceMethod<void>("Cbk_WriteLog",
                                    "(Ljava/lang/String;Ljava/lang/String;)V",
                                    *levelStr, *msgStr);
    }

private:
    static std::unique_ptr<JavaLogger> instance;

    Log::Level thresholdLevel;
    Log::LogFunc oldLogFunc;

    JavaLogger(Log::Level level):
        thresholdLevel(level)
    {}

    static Log::Level
    LevelFromString(const std::string &name)
    {
        if (name == "TRACE" ||
            name == "DEBUG" ||
            name == "ALL") {
            return Log::Level::DEBUG;
        } else if (name == "INFO") {
            return Log::Level::INFO;
        } else if (name == "WARN") {
            return Log::Level::WARNING;
        } else if (name == "ERROR") {
            return Log::Level::ERROR;
        } else if (name == "FATAL" || name == "OFF") {
            return Log::Level::CRITICAL;
        }
        ADK_EXCEPTION(InternalErrorException,
                      "Unrecognized log level: " << name);
    }

    static const char *
    LevelToString(Log::Level level)
    {
        switch (level) {
        case Log::Level::DEBUG:
            return "DEBUG";
        case Log::Level::INFO:
            return "INFO";
        case Log::Level::WARNING:
            return "WARN";
        case Log::Level::ERROR:
            return "ERROR";
        case Log::Level::CRITICAL:
            return "CRITICAL";
        default:
            ADK_EXCEPTION(InternalErrorException, "Unknown log level");
        }
    }
};

std::unique_ptr<JavaLogger> JavaLogger::instance;

} /* anonymous namespace */

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
                             const std::string &msg):
    Exception(file, line, msg), excClass(excClass)
{
    ADK_WARNING("Exception in native code: %s", this->_msg.c_str());
}

JavaException::JavaException(const char *file, int line, const std::string &msg):
    JavaException(file, line, "com/ast/utils/NativeComponent$NativeException", msg)
{}

JavaException::JavaException(const char *file, int line,
                             const char *excClassName, const std::string &msg):
    JavaException(file, line, ADK_JNI_CALL(FindClass, excClassName), msg)
{}

void
JavaException::Throw()
{
    if (jniEnv->ThrowNew(excClass, _msg.c_str())) {
        printf("ThrowNew() failed for exception: %s\n", _msg.c_str());
    }
}

void JNICALL
Java_com_ast_utils_NativeComponent_Initialize
    (JNIEnv *env, jobject ifaceObj, jstring logLevel)
ADK_JNI_METHOD_START(env) {

    Java::Initialize(env, ifaceObj);
    JavaLogger::Initialize(Java::GetString(logLevel));
    ADK_INFO("Native component initialized");

} ADK_JNI_METHOD_END(void)


JNIEXPORT void JNICALL
Java_com_ast_utils_NativeComponent_Terminate
    (JNIEnv *env, jobject)
ADK_JNI_METHOD_START(env) {

    ADK_INFO("Native component terminated");
    JavaLogger::Shutdown();

} ADK_JNI_METHOD_END(void)

