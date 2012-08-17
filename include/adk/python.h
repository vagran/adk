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

/* Automatically generated files for corresponding .py files. */
#include <auto_adk_py.h>

/** Start of embedded Python file. */
#define ADK_PY_FILE_START(__fileName) &_binary_ ## __fileName ## _py_start
/** End of embedded Python file. */
#define ADK_PY_FILE_END(__fileName) &_binary_ ## __fileName ## _py_end

/** Create string with embedded Python file content.
 * @param __fileName Name of .py file.
 * For example, if file was named "example.py" then this macro should be
 * used as in the example:
 * @code
 * py::Run(ADK_PY_FILE(example));
 * @endcode
 */
#define ADK_PY_FILE(__fileName) \
    std::string(ADK_PY_FILE_START(__fileName), \
                ADK_PY_FILE_END(__fileName) - ADK_PY_FILE_START(__fileName))

namespace adk {

namespace py {

/** When Python produces an exception it is wrapped by this class. */
class Exception: public adk::Exception {
private:
    PyObject *_excType, *_excValue, *_traceback;

    static std::string
    Describe(PyObject *excType, PyObject *excValue, PyObject *traceback);

    Exception(PyObject *excType, PyObject *excValue, PyObject *traceback):
        adk::Exception(Describe(excType, excValue, traceback)),
        _excType(excType), _excValue(excValue), _traceback(traceback)
    {
    }

#   ifdef DEBUG
    Exception(const char *file, int line,
              PyObject *excType, PyObject *excValue, PyObject *traceback):
              adk::Exception(file, line, Describe(excType, excValue, traceback)),
              _excType(excType), _excValue(excValue), _traceback(traceback)
    {
    }
#   endif /* DEBUG */

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
    Fetch(const char *file = nullptr, int line = 0)
    {
        if (PyErr_Occurred()) {
            PyObject *excType, *excValue, *traceback;
            PyErr_Fetch(&excType, &excValue, &traceback);
            PyErr_Clear();
            PyErr_NormalizeException(&excType, &excValue, &traceback);
            if (file) {
                return Exception(file, line, excType, excValue, traceback);
            }
            return Exception(excType, excValue, traceback);
        }
        return Exception(nullptr, nullptr, nullptr);
    }

    /** Test if exception was fetched. */
    operator bool()
    {
        return _excType != nullptr;
    }
};

#ifdef DEBUG
#define __ADK_PY_FETCH_EXCEPTION() adk::py::Exception::Fetch(__FILE__, __LINE__)
#else /* DEBUG */
#define __ADK_PY_FETCH_EXCEPTION() adk::py::Exception::Fetch()
#endif /* DEBUG */

/** This macro can be for checking and fetching Python exception if any has
 * occurred. Python exception is transformed to C++ exception and is thrown as
 * usually.
 */
#define ADK_PY_CHECK_EXCEPTION() do { \
    if (UNLIKELY(PyErr_Occurred())) { \
        throw __ADK_PY_FETCH_EXCEPTION(); \
    } \
} while (false)

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
        obj._obj = nullptr;
    }

    Object(int value)
    {
        _obj = PyLong_FromLong(value);
    }

    Object(long value)
    {
        _obj = PyLong_FromLong(value);
    }

    Object(double value)
    {
        _obj = PyFloat_FromDouble(value);
    }

    Object(const char *value)
    {
        _obj = PyUnicode_FromString(value);
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
            ADK_PY_CHECK_EXCEPTION();
        }
        return obj;
    }

    /** Return reference to None object. */
    static Object
    None()
    {
        return Object(Py_None, false);
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
     * @return Pointer to the object.
     */
    PyObject *
    Steal()
    {
        PyObject *obj = _obj;
        _obj = nullptr;
        return obj;
    }

    /** Get reference to the object.
     *
     * @param newRef Return new reference if @a true, borrowed reference if
     *      @a false.
     * @return Pointer to the object.
     */
    PyObject *
    Get(bool newRef = false) const
    {
        if (_obj && newRef) {
            Py_INCREF(_obj);
        }
        return _obj;
    }

    bool
    HasAttr(const char *attrName) const
    {
        return PyObject_HasAttrString(_obj, attrName);
    }

    /** Get object attribute by its name. */
    Object
    GetAttr(const char *attrName) const
    {
        Object res(PyObject_GetAttrString(_obj, attrName));
        if (!res) {
            ADK_PY_CHECK_EXCEPTION();
        }
        return res;
    }

    void
    SetAttr(const char *attrName, const Object &attrValue)
    {
        if (UNLIKELY(PyObject_SetAttrString(_obj, attrName, attrValue.Get()) == -1)) {
            ADK_PY_CHECK_EXCEPTION();
        }
    }

    void
    DelAttr(const char *attrName)
    {
        if (UNLIKELY(PyObject_DelAttrString(_obj, attrName) == -1)) {
            ADK_PY_CHECK_EXCEPTION();
        }
    }

    std::string
    Str() const;

    std::string
    Repr() const;

    long
    Int() const
    {
        long res = PyLong_AsLong(_obj);
        ADK_PY_CHECK_EXCEPTION();
        return res;
    }

    double
    Float() const
    {
        double res = PyFloat_AsDouble(_obj);
        ADK_PY_CHECK_EXCEPTION();
        return res;
    }

    Py_ssize_t
    Length() const
    {
        Py_ssize_t len = PyObject_Length(_obj);
        if (UNLIKELY(len == -1)) {
            ADK_PY_CHECK_EXCEPTION();
        }
        return len;
    }

    Object
    GetItem(const Object &key) const
    {
        Object item(PyObject_GetItem(_obj, key.Get()));
        if (UNLIKELY(!item)) {
            ADK_PY_CHECK_EXCEPTION();
        }
        return item;
    }

    Object
    GetItem(const char *key) const;

    Object
    operator [](const Object &key) const
    {
        return GetItem(key);
    }

    Object
    operator [](const char *key) const
    {
        return GetItem(key);
    }

    void
    SetItem(const Object &key, const Object &value)
    {
        if (UNLIKELY(PyObject_SetItem(_obj, key.Get(), value.Get()) == -1)) {
            ADK_PY_CHECK_EXCEPTION();
        }
    }

    void
    DetItem(const Object &key)
    {
        if (UNLIKELY(PyObject_DelItem(_obj, key.Get()) == -1)) {
            ADK_PY_CHECK_EXCEPTION();
        }
    }

    /** Return type object. */
    Object
    Type() const
    {
        Object res(PyObject_Type(_obj));
        if (UNLIKELY(!res)) {
            ADK_PY_CHECK_EXCEPTION();
        }
        return res;
    }

    /** Check if the object is "None" */
    bool
    IsNone() const
    {
        return _obj == Py_None;
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
            ADK_PY_CHECK_EXCEPTION();
        }
        Object res(PyObject_CallObject(_obj, argsObj.Get()));
        if (!res) {
            ADK_PY_CHECK_EXCEPTION();
        }
        return res;
    }

    /** Call object method with specified name.
     *
     * @param name Method name.
     * @param args Arguments (of type 'const Object &') to the method.
     * @return Method return value.
     */
    template <class... Args>
    Object
    CallMethod(const char *name, const Args&... args)
    {
        Object method(GetAttr(name));
        if (UNLIKELY(!method)) {
            ADK_PY_CHECK_EXCEPTION();
        }
        return method(args...);
    }

    /** Parse arguments tuple.
     *
     * @param func Name of calling function.
     * @param file Source file name.
     * @param line Line number in source file.
     * @param maxArgs Maximal number of arguments.
     * @param minArgs Minimal number of arguments.
     * @return Vector with arguments. TypeError Python exception is raised in
     *      case of error. The caller should check this by PyErr_Occurred
     *      function and return @a nullptr from the method if error occurred.
     */
    std::vector<Object>
    ParseArguments(const char *func, const char *file, int line,
                   int maxArgs = -1, int minArgs = -1) const
    {
        ASSERT(PyTuple_Check(_obj));
        Py_ssize_t size = PyTuple_Size(_obj);
        if (minArgs != -1 && size < minArgs) {
            if (func) {
                PyErr_Format(PyExc_TypeError,
                             "[%s:%d] Function '%s()' expects at least %d arguments (%d given)",
                             file, line, func, minArgs, size);
            } else {
                PyErr_Format(PyExc_TypeError,
                             "The function expects at least %d arguments (%d given)",
                             minArgs, size);
            }
            return std::vector<Object>();
        }
        if (maxArgs != -1 && size > maxArgs) {
            if (file) {
                PyErr_Format(PyExc_TypeError,
                             "[%s:%d] Function '%s()' expects at most %d arguments (%d given)",
                             file, line, func, maxArgs, size);
            } else {
                PyErr_Format(PyExc_TypeError,
                             "The function expects at most %d arguments (%d given)",
                             maxArgs, size);
            }
            return std::vector<Object>();
        }
        std::vector<Object> result(size);
        for (Py_ssize_t i = 0; i < size; i++) {
            result[i] = PyTuple_GetItem(_obj, i);
        }
        return result;
    }

    std::vector<Object>
    ParseArguments(int maxArgs = -1, int minArgs = -1) const
    {
        return ParseArguments(nullptr, nullptr, 0, maxArgs, minArgs);
    }
};

/** Convenience wrapper function for @ref Object::ParseArguments method. */
template <class... Args>
std::vector<Object>
ParseArguments(PyObject *obj, Args&&... args)
{
    return Object(obj, false).ParseArguments(std::forward<Args>(args)...);
}

template <class... Args>
std::vector<Object>
ParseArguments(const Object &obj, Args&&... args)
{
    return obj.ParseArguments(std::forward<Args>(args)...);
}

#define ADK_PY_PARSE_ARGUMENTS(obj, ...) \
    adk::py::ParseArguments(obj, __FUNCTION__, __FILE__, __LINE__, ## __VA_ARGS__)

/** Wrapper for Python sequence protocol API. */
class ObjectSequence: public Object {
public:
    ObjectSequence(PyObject *obj = nullptr, bool isNew = true): Object(obj, isNew)
    {
        ASSERT(!_obj || PySequence_Check(_obj));
    }

    ObjectSequence(const Object &obj): Object(obj)
    {
        ASSERT(!_obj || PySequence_Check(_obj));
    }

    ObjectSequence(Object &&obj): Object(std::move(obj))
    {
        ASSERT(!_obj || PySequence_Check(_obj));
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

class ObjectUnicode: public ObjectSequence {
public:
    ObjectUnicode(PyObject *obj = nullptr, bool isNew = true): ObjectSequence(obj, isNew)
    {
        ASSERT(!_obj || PyUnicode_Check(_obj));
    }

    ObjectUnicode(const Object &obj): ObjectSequence(obj)
    {
        ASSERT(!_obj || PyUnicode_Check(_obj));
    }

    ObjectUnicode(Object &&obj): ObjectSequence(std::move(obj))
    {
        ASSERT(!_obj || PyUnicode_Check(_obj));
    }

    ObjectUnicode(const char *s): ObjectSequence(PyUnicode_FromString(s))
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

class ObjectDict: public Object {
public:
    ObjectDict(PyObject *obj = nullptr, bool isNew = true): Object(obj, isNew)
    {
        ASSERT(!_obj || PyDict_Check(_obj));
    }

    ObjectDict(const Object &obj): Object(obj)
    {
        ASSERT(!_obj || PyDict_Check(_obj));
    }

    ObjectDict(Object &&obj): Object(std::move(obj))
    {
        ASSERT(!_obj || PyDict_Check(_obj));
    }

    /** Create and return new empty dictionary. */
    static ObjectDict
    New()
    {
        return ObjectDict(PyDict_New());
    }

    void
    Clear()
    {
        PyDict_Clear(_obj);
    }

    bool
    Contains(const Object &key)
    {
        int res = PyDict_Contains(_obj, key.Get());
        if (UNLIKELY(res == -1)) {
            ADK_PY_CHECK_EXCEPTION();
        }
        return res;
    }

    /** Get "builtins" dictionary. */
    static ObjectDict
    Builtins()
    {
        return ObjectDict(PyEval_GetBuiltins(), false);
    }
};

class ObjectModule: public Object {
public:
    /** The constructor imports the module with the specified name. */
    ObjectModule(const char *name):
        Object(PyImport_ImportModule(name))
    {

    }

    /** This constructor creates new module. */
    ObjectModule(PyModuleDef *def):
        Object(PyModule_Create(def))
    {

    }

    /** Add object to the module. */
    void
    AddObject(const char *name, const Object &obj)
    {
        if (UNLIKELY(PyModule_AddObject(_obj, name, obj.Get(true)) == -1)) {
            ADK_PY_CHECK_EXCEPTION();
        }
    }
};

/** Run Python code from the specified string.
 * @param s String with the code.
 * @param locals Local namespace dictionary.
 * @param globals Global namespace dictionary.
 * @param start Starting token.
 * @param flags Optional compiler flags.
 * @return Resulted Python object.
 */
Object
Run(const std::string &s,
    Object locals = ObjectDict::New(), Object globals = ObjectDict::New(),
    int start = Py_file_input,
    PyCompilerFlags *flags = nullptr);

} /* namespace py */

} /* namespace adk */

#endif /* ADK_PYTHON_H_ */
