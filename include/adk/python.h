/* /ADK/include/adk/types.h
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file types.h
 * ADK common data types.
 */

#ifndef ADK_PYTHON_H_
#define ADK_PYTHON_H_

namespace py {

/** Convenience class for global Python interpreter initialization and
 * finalization.
 */
class Interpreter {
public:
    Interpreter()
    {
        Py_InitializeEx(0);
    }

    ~Interpreter()
    {
        Py_Finalize();
    }
};

/** Convenience class for releasing Python global interpreter lock when
 * executing potentially blocking operations.
 * Usage:
 * @code
 * {
 *     BlockingSection bs;
 *     // do some blocking operation
 * }
 * @endcode
 */
class BlockingSection {
private:
    PyThreadState *_save;
public:
    BlockingSection(const BlockingSection &) = delete;
    BlockingSection()
    {
        _save = PyEval_SaveThread();
    }

    ~BlockingSection()
    {
        PyEval_RestoreThread(_save);
    }
};

/** Convenience class for calling Python API from non-Python threads (e.g. some
 * callbacks invoking).
 * Usage:
 * @code
 * {
 *     ThreadLock lock;
 *     // call some Python API
 * }
 * @endcode
 */
class ThreadLock {
private:
    PyGILState_STATE _state;
public:
    ThreadLock(const ThreadLock &) = delete;
    ThreadLock()
    {
        _state = PyGILState_Ensure();
    }

    ~ThreadLock()
    {
        PyGILState_Release(_state);
    }
};

/** Wrapper class for PyObject type. */
class Object {
private:
    PyObject *_obj;
public:
    /** Initialize wrapped by object pointer.
     *
     * @param obj Target object.
     * @param isNew @a true if object is newly created and reference ownership
     *      is transferred to the calling code. @a false if the reference is
     *      borrowed.
     */
    Object(PyObject *obj, bool isNew = true): _obj(obj)
    {
        if (_obj && !isNew) {
            Py_INCREF(_obj);
        }
    }

    Object(const Object &obj): _obj(obj._obj)
    {
        if (_obj) {
            Py_INCREF(_obj);
        }
    }

    Object(Object &&obj): _obj(obj._obj)
    {
    }

    /** Assignment is always treated as borrowed reference. If it is not, the
     * following pattern can be used:
     * @code
     * myobj = py::Object(SomeObjectCreationFunction());
     * @endcode
     * which will lead to move assignment operator invocation.
     *
     * @param obj Target object.
     * @return Self reference.
     */
    Object &
    operator =(PyObject *obj)
    {
        if (_obj) {
            Py_DECREF(_obj);
        }
        _obj = obj;
        Py_INCREF(_obj);
        return *this;
    }

    Object &
    operator =(const Object &obj)
    {
        if (_obj) {
            Py_DECREF(_obj);
        }
        _obj = obj._obj;
        Py_INCREF(_obj);
        return *this;
    }

    Object &
    operator =(Object &&obj)
    {
        if (_obj) {
            Py_DECREF(_obj);
        }
        _obj = obj._obj;
        return *this;
    }

    ~Object()
    {
        if (_obj) {
            Py_DECREF(_obj);
        }
    }

    /** Wrapper for @a Py_VaBuildValue function. */
    static Object
    Build(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        Object obj(Py_VaBuildValue(fmt, args));
        va_end(args);
        return obj;
    }

    bool
    operator ==(const Object &obj)
    {
        return obj._obj == _obj;
    }

    operator bool()
    {
        return _obj != nullptr;
    }

    /** Steal reference to the object. The result should be passed to some
     * function which steals the object reference.
     * @return Pointer to object.
     */
    PyObject *
    Steal()
    {
        PyObject *obj = _obj;
        _obj = nullptr;
        return obj;
    }

    PyObject *
    Get() const
    {
        return _obj;
    }
};

/** Run Python code from the specified string.
 * XXX exceptions
 * @param s String with the code.
 * @return Resulted Python object.
 */
Object
Run(const std::string &s);

} /* namespace py */

#endif /* ADK_PYTHON_H_ */
