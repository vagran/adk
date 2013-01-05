/* /ADK/include/adk/exception.h
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file exception.h
 * ADK exception base class.
 */

#ifndef ADK_EXCEPTION_H_
#define ADK_EXCEPTION_H_

namespace adk {

/** Base class for all ADK and its client code exceptions. */
class Exception: public std::exception {
protected:
#   ifdef DEBUG
    /** Source file where the exception occurred. */
    const char *_file;
    /** Line number in the source file where the exception occurred. */
    int _line;
#   endif /* DEBUG */
    /** Exception message. */
    std::string _msg;
private:

#   ifdef DEBUG
    void
    _StrFileLine()
    {
        std::stringstream ss;
        ss << "[" << _file << ":" << _line << "]: ";
        ss << _msg;
        _msg = ss.str();
    }
#   endif /* DEBUG */

public:
    Exception(const char *msg): _msg(msg) {}

    Exception(const std::string &msg): _msg(msg) {}

#   ifdef DEBUG
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
#   endif /* DEBUG */

    virtual
    ~Exception() noexcept
    {}

    virtual const char *
    what() const noexcept
    {
        return _msg.c_str();
    }
};

#ifdef DEBUG
#define __ADK_THROW_EXCEPTION(__exception, __msg, ...) \
    throw __exception(__FILE__, __LINE__, __msg, ## __VA_ARGS__)
#else /* DEBUG */
#define __ADK_THROW_EXCEPTION(__exception, __msg, ...) \
    throw __exception(__msg, ## __VA_ARGS__)
#endif /* DEBUG */

/** Throw ADK exception.
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

} /* namespace adk */

#endif /* ADK_EXCEPTION_H_ */
