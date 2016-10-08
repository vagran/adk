/* This file is a part of ADK library.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See LICENSE file for copyright details.
 */

/** @file exception.h
 * ADK exception base class.
 */

#ifndef ADK_EXCEPTION_H_
#define ADK_EXCEPTION_H_

namespace adk {

/** Base class for all ADK and its client code exceptions. */
class Exception: public std::exception {
public:
    Exception(const char *msg): _msg(msg)
    {}

    Exception(const std::string &msg): _msg(msg)
    {}

    Exception(const char *file, int line, const char *msg):
        _file(file), _line(line), _msg(msg)
    {
        _StrFileLine();
    }

    Exception(const char *file, int line, const std::string &msg):
        _file(file), _line(line), _msg(msg)
    {
        _StrFileLine();
    }

    virtual
    ~Exception() noexcept
    {}

    virtual const char *
    what() const noexcept override
    {
        return _msg.c_str();
    }
protected:
    /** Source file where the exception occurred. */
    const char *_file = nullptr;
    /** Line number in the source file where the exception occurred. */
    int _line = 0;
    /** Exception message. */
    std::string _msg;
private:

    void
    _StrFileLine();
};

#define __ADK_THROW_EXCEPTION(__exception, __msg, ...) \
    throw __exception(__FILE__, __LINE__, __msg, ## __VA_ARGS__)

/** Throw ADK exception. Standard ADK exceptions can be easily extended by user
 * defined exception. See example @ref ADK_USB_EXCEPTION and @ref LibusbException.
 *
 * @param __exception Exception class.
 * @param __msg Message which could be streaming expression. Additional arguments
 *      to the exception class constructor can be specified after the message.
 *
 * Usage example:
 * @code
 * ADK_EXCEPTION(Exception, "Error " << errno << ", will now terminate.")
 * @endcode
 */
#define ADK_EXCEPTION(__exception, __msg, ...) do { \
    std::stringstream __ss; \
    __ss << __msg; \
    __ADK_THROW_EXCEPTION(__exception, __ss.str(), ## __VA_ARGS__); \
} while (false)

namespace internal {

namespace {

template <typename T>
inline std::string
PrintExcParam(T param)
{
    std::stringstream ss;
    ss << ": " << "[";
    param.ToString(ss);
    ss << "]";
    return ss.str();
}

/** Define string conversion method for trivial types. */
#define __ADK_PARAM_EXC_PRINT_FUNC(__type) \
    template <> \
    inline std::string \
    PrintExcParam<__type>(__type param) \
    { \
        std::stringstream ss; \
        ss << ": " << "[" << param << "]"; \
        return ss.str(); \
    }

__ADK_PARAM_EXC_PRINT_FUNC(bool);
__ADK_PARAM_EXC_PRINT_FUNC(char);
__ADK_PARAM_EXC_PRINT_FUNC(unsigned char);
__ADK_PARAM_EXC_PRINT_FUNC(short);
__ADK_PARAM_EXC_PRINT_FUNC(unsigned short);
__ADK_PARAM_EXC_PRINT_FUNC(int);
__ADK_PARAM_EXC_PRINT_FUNC(unsigned int);
__ADK_PARAM_EXC_PRINT_FUNC(long);
__ADK_PARAM_EXC_PRINT_FUNC(unsigned long);
__ADK_PARAM_EXC_PRINT_FUNC(long long);
__ADK_PARAM_EXC_PRINT_FUNC(unsigned long long);

} /* anonymous namespace */
} /* namespace internal */

/** Exception with parameter. Parameter can be built-in type or a class which
 * should have @a ToString() method with the following signature:
 * @code
 * void ToString(std::stringstream &ss);
 * @endcode
 * It should write to the provided stream human readable representation of the
 * parameter value.
 *
 * Usage example:
 * @code
 * typedef adk::ParamException<MyParamType> MyException;
 *
 * ADK_EXCEPTION(MyException, "message " << someValue, myParam);
 * @endcode
 */
template <class Base, typename TParam>
class ParamException: public Base {
public:
    template<class TParamArg>
    ParamException(const char *msg, TParamArg &&param):
        Base(msg), _param(std::forward<TParamArg>(param))
    {
        _AppendParamStr();
    }

    template<class TParamArg>
    ParamException(const std::string &msg, TParamArg &&param):
        Base(msg), _param(std::forward<TParamArg>(param))
    {
        _AppendParamStr();
    }

    template<class TParamArg>
    ParamException(const char *file, int line, const char *msg, TParamArg &&param):
        Base(file, line, msg), _param(std::forward<TParamArg>(param))
    {
        _AppendParamStr();
    }

    template<class TParamArg>
    ParamException(const char *file, int line, const std::string &msg, TParamArg &&param):
        Base(file, line, msg), _param(std::forward<TParamArg>(param))
    {
        _AppendParamStr();
    }

    /** Get associated parameter. */
    const TParam &
    GetParam() const
    {
        return _param;
    }

    /** Get associated parameter. */
    TParam &
    GetParam()
    {
        return _param;
    }
protected:
    TParam _param;
private:
    void
    _AppendParamStr()
    {
        Base::_msg += internal::PrintExcParam(_param);
    }
};


/** Define custom exception which is just another subclass from adk::Exception
 * with the same functionality.
 * @param __clsName Class name for new exception type.
 */
#define ADK_DEFINE_EXCEPTION(__clsName) \
    class __clsName: public adk::Exception { \
    public: \
        __clsName(const char *msg): adk::Exception(msg) {} \
        __clsName(const std::string &msg): adk::Exception(msg) {} \
        __clsName(const char *file, int line, const char *msg): \
            adk::Exception(file, line, msg) {} \
        __clsName(const char *file, int line, const std::string &msg): \
            adk::Exception(file, line, msg) {} \
    };

/** Define custom exception derived from another exception class. */
#define ADK_DEFINE_DERIVED_EXCEPTION(__clsName, __baseCls) \
    class __clsName: public __baseCls { \
    public: \
        using __baseCls::__baseCls; \
    };


/** Define custom exception with parameter. See @ref ParamException. See @ref
 * LibusbException for example.
 * @param __clsName Class name for new exception type.
 * @param __paramType Type name for the parameter.
 */
#define ADK_DEFINE_PARAM_EXCEPTION(__clsName, __paramType) \
    class __clsName: public adk::ParamException<adk::Exception, __paramType> { \
    public: \
        template<class TParamArg> \
        __clsName(const char *msg, TParamArg &&param): \
            adk::ParamException<adk::Exception, __paramType>(msg, std::forward<TParamArg>(param)) {} \
        \
        template<class TParamArg> \
        __clsName(const std::string &msg, TParamArg &&param): \
            adk::ParamException<adk::Exception, __paramType>(msg, std::forward<TParamArg>(param)) {} \
        \
        template<class TParamArg> \
        __clsName(const char *file, int line, const char *msg, TParamArg &&param): \
            adk::ParamException<adk::Exception, __paramType>(file, line, msg, std::forward<TParamArg>(param)) {} \
        \
        template<class TParamArg> \
        __clsName(const char *file, int line, const std::string &msg, TParamArg &&param): \
            adk::ParamException<adk::Exception, __paramType>(file, line, msg, std::forward<TParamArg>(param)) {} \
    };

#define ADK_DEFINE_DERIVED_PARAM_EXCEPTION(__clsName, __baseCls, __paramType) \
    class __clsName: public adk::ParamException<__baseCls, __paramType> { \
    public: \
        template<class TParamArg> \
        __clsName(const char *msg, TParamArg &&param): \
            adk::ParamException<__baseCls, __paramType>(msg, std::forward<TParamArg>(param)) {} \
        \
        template<class TParamArg> \
        __clsName(const std::string &msg, TParamArg &&param): \
            adk::ParamException<__baseCls, __paramType>(msg, std::forward<TParamArg>(param)) {} \
        \
        template<class TParamArg> \
        __clsName(const char *file, int line, const char *msg, TParamArg &&param): \
            adk::ParamException<__baseCls, __paramType>(file, line, msg, std::forward<TParamArg>(param)) {} \
        \
        template<class TParamArg> \
        __clsName(const char *file, int line, const std::string &msg, TParamArg &&param): \
            adk::ParamException<__baseCls, __paramType>(file, line, msg, std::forward<TParamArg>(param)) {} \
    };

/** Generic exception thrown when invalid parameter is specified. */
ADK_DEFINE_EXCEPTION(InvalidParamException);
/** Generic exception thrown when operation is executed in invalid state. */
ADK_DEFINE_EXCEPTION(InvalidOpException);
/** Generic exception thrown when operation is executed in invalid state. */
ADK_DEFINE_EXCEPTION(InternalErrorException);

/** Exception for failed system calls. */
ADK_DEFINE_PARAM_EXCEPTION(SysException, int);

#define ADK_SYS_EXCEPTION(__msg) do { \
    std::stringstream __ss; \
    int __code = adk::GetSystemErrorCode(); \
    __ss << "System error: [" << __code << "]: " << \
        adk::GetSystemError() << ": " << __msg; \
    __ADK_THROW_EXCEPTION(adk::SysException, __ss.str(), __code); \
} while (false)

} /* namespace adk */

#endif /* ADK_EXCEPTION_H_ */
