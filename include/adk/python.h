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
protected:
    PyObject *_obj;
public:
    /** Initialize wrapped by object pointer.
     *
     * @param obj Target object.
     * @param isNew @a true if object is newly created and reference ownership
     *      is transferred to the calling code. @a false if the reference is
     *      borrowed.
     */
    Object(PyObject *obj = nullptr, bool isNew = true): _obj(obj)
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
        if (!obj) {
            /* Exception occurred. */
            //XXX
        }
        return obj;
    }

    /** Return reference to None object. */
    static Object
    None()
    {
        return Object(Py_None);
    }

    bool
    operator ==(const Object &obj) const
    {
        return obj._obj == _obj;
    }

    operator bool() const
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

    /** Call callable object. Arguments should be @ref Object class
     * references.
     */
    template <class... Args>
    Object
    operator ()(const Args&... args) const
    {
        ASSERT(PyCallable_Check(_obj));
        Object argsObj(PyTuple_Pack(sizeof...(Args), args.Get()...));
        if (!argsObj) {
            //XXX
        }
        Object res(PyObject_CallObject(_obj, argsObj.Get()));
        if (!res) {
            /* Exception occurred. */
            //XXX
        }
        return res;
    }

    /** Get object attribute by its name. */
    Object
    Attr(const char *attrName) const
    {
        Object res(PyObject_GetAttrString(_obj, attrName));
        if (!res) {
            /* Exception occurred. */
            //XXX
        }
        return res;
    }
};

/** Wrapper for Python sequence protocol API. */
class ObjectSequence: public Object {
public:
    ObjectSequence(PyObject *obj = nullptr, bool isNew = true): Object(obj, isNew)
    {
    }

    ObjectSequence(const Object &obj): Object(obj)
    {
    }

    ObjectSequence(Object &&obj): Object(std::move(obj))
    {
    }

    class Iterator {
    private:
        const ObjectSequence &_obj;
        Py_ssize_t _idx;
    public:
        Iterator(const ObjectSequence &obj, Py_ssize_t idx): _obj(obj), _idx(idx)
        {}

        void
        operator ++()
        {
            _idx++;
        }

        Object
        operator *() const
        {
            return Object(PySequence_GetItem(_obj.Get(), _idx));
        }

        bool
        operator !=(const Iterator &it) const
        {
            return it._idx != _idx;
        }
    };

    /** Get number of elements in the sequence. */
    Py_ssize_t
    Size() const
    {
        return PySequence_Size(_obj);
    }

    Iterator
    begin() const
    {
        return Iterator(*this, 0);
    }

    Iterator
    end() const
    {
        return Iterator(*this, Size());
    }
};

class ObjectUnicode: public Object {
public:
    ObjectUnicode(PyObject *obj = nullptr, bool isNew = true): Object(obj, isNew)
    {
    }

    ObjectUnicode(const Object &obj): Object(obj)
    {
    }

    ObjectUnicode(Object &&obj): Object(std::move(obj))
    {
    }

    ObjectUnicode(const char *s): Object(PyUnicode_FromString(s))
    {
    }

    std::string
    GetString() const
    {
        ASSERT(PyUnicode_Check(_obj));
        Object bytesObj(PyUnicode_AsUTF8String(_obj));
        return std::string(PyBytes_AsString(bytesObj.Get()));
    }
};

class ObjectModule: public Object {
public:
    /** The constructor imports the module with the specified name. */
    ObjectModule(const char *name):
        Object(PyImport_ImportModule(name))
    {

    }
};

/** When Python produces an exception it is wrapped by this class. */
class Exception: public adk::Exception {
private:
    Object _excType, _excValue, _traceback;

    static std::string
    Describe(const Object &_excType, const Object &_excValue, const Object &_traceback)
    {
        ObjectModule tbMod("traceback");
        Object formatFunc(tbMod.Attr("format_exception"));
        ObjectSequence result(formatFunc(_excType, _excValue,
                                         _traceback ? _traceback : Object::None()));
        for (auto line: result) {
            ObjectUnicode s(line);
            ADK_INFO("%s", s.GetString().c_str());
        }
        return std::string("xxx");
    }

    Exception(PyObject *excType, PyObject *excValue, PyObject *traceback):
        adk::Exception(Describe(Object(excType, false),
                                Object(excValue, false),
                                Object(traceback, false))),
        _excType(excType, false), _excValue(excValue, false),
        _traceback(traceback, false)
    {
    }

public:
    virtual
    ~Exception() noexcept
    {}

    /** Fetch active error indication. The error is cleared after that.
     * Empty object is created if no active error (can be tested by bool
     * operator).
     * @return Exception object.
     */
    static Exception
    Fetch()
    {
        if (PyErr_Occurred()) {
            PyObject *excType, *excValue, *traceback;
            PyErr_Fetch(&excType, &excValue, &traceback);
            PyErr_Clear();
            PyErr_NormalizeException(&excType, &excValue, &traceback);
            return Exception(excType, excValue, traceback);
        }
        return Exception(nullptr, nullptr, nullptr);
    }

    std::string
    Describe()
    {
        return Describe(_excType, _excValue, _traceback);
    }

    /** Test if exception was fetched. */
    operator bool()
    {
        return _excType;
    }
};

/** Run Python code from the specified string.
 * XXX exceptions
 * @param s String with the code.
 * @return Resulted Python object.
 */
Object
Run(const std::string &s, int start = Py_file_input, Object globals = Object(),
    Object locals = Object(), PyCompilerFlags *flags = nullptr);

} /* namespace py */

#endif /* ADK_PYTHON_H_ */
