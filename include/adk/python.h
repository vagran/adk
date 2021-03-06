/* This file is a part of ADK library.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See LICENSE file for copyright details.
 */

/** @file types.h
 * ADK common data types.
 */

#ifndef ADK_PYTHON_H_
#define ADK_PYTHON_H_

namespace adk {

/** Start of embedded Python file. */
#define ADK_PY_FILE_START(__fileName) &adk::_binary_ ## __fileName ## _py_start
/** End of embedded Python file. */
#define ADK_PY_FILE_END(__fileName) &adk::_binary_ ## __fileName ## _py_end

/** Create string with embedded Python file content.
 * @param __fileName Name of .py file.
 * For example, if file was named "example.py" then this macro should be
 * used as in the example:
 * @code
 * py::Run(ADK_PY_FILE(example));
 * @endcode
 */
#define ADK_PY_FILE(__fileName) ({ \
    adk::ResourceDesc res = adk::GetResource(ADK_STR(__fileName) ".py"); \
    res.GetString(); \
})

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
    Fetch(
#ifdef DEBUG
          const char *file = nullptr, int line = 0
#endif /* DEBUG */
          )
    {
        if (PyErr_Occurred()) {
            PyObject *excType, *excValue, *traceback;
            PyErr_Fetch(&excType, &excValue, &traceback);
            PyErr_Clear();
            PyErr_NormalizeException(&excType, &excValue, &traceback);
#           ifdef DEBUG
            if (file) {
                return Exception(file, line, excType, excValue, traceback);
            }
#           endif /* DEBUG */
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
private:
    static std::atomic<int> _refCount;
public:
    Interpreter()
    {
        if (_refCount.fetch_add(1) == 0) {
            Py_InitializeEx(0);
        }
    }

    ~Interpreter()
    {
        if (_refCount.fetch_sub(1) == 1) {
            Py_Finalize();
        }
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

    Object(std::nullptr_t):
        _obj(nullptr)
    {}

    Object(bool value)
    {
        _obj = PyBool_FromLong(value);
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

    Object(const std::string &value)
    {
        _obj = PyUnicode_FromString(value.c_str());
    }

    Object(const char *value)
    {
        _obj = PyUnicode_FromString(value);
    }

    Object &
    operator =(const Object &obj)
    {
        if (UNLIKELY(_obj == obj._obj)) {
            return *this;
        }
        if (_obj) {
            Py_DECREF(_obj);
        }
        if (obj._obj) {
            Py_INCREF(_obj);
        }
        _obj = obj._obj;
        return *this;
    }

    Object &
    operator =(Object &&obj)
    {
        if (UNLIKELY(&obj == this)) {
            return *this;
        }
        if (_obj) {
            Py_DECREF(_obj);
        }
        _obj = obj._obj;
        obj._obj = nullptr;
        return *this;
    }

    ~Object()
    {
        if (LIKELY(_obj)) {
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
        if (UNLIKELY(!obj)) {
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
    HasAttr(const std::string &attrName) const
    {
        return PyObject_HasAttrString(_obj, attrName.c_str());
    }

    /** Get object attribute by its name. */
    Object
    GetAttr(const std::string &attrName) const
    {
        Object res(PyObject_GetAttrString(_obj, attrName.c_str()));
        if (UNLIKELY(!res)) {
            ADK_PY_CHECK_EXCEPTION();
        }
        return res;
    }

    void
    SetAttr(const std::string &attrName, const Object &attrValue)
    {
        if (UNLIKELY(PyObject_SetAttrString(_obj, attrName.c_str(),
                                            attrValue.Get()) == -1)) {
            ADK_PY_CHECK_EXCEPTION();
        }
    }

    void
    DelAttr(const std::string &attrName)
    {
        if (UNLIKELY(PyObject_DelAttrString(_obj, attrName.c_str()) == -1)) {
            ADK_PY_CHECK_EXCEPTION();
        }
    }

    std::string
    Str() const;

    std::string
    Repr() const;

    bool
    Bool() const
    {
        bool res = PyObject_IsTrue(_obj) != 0;
        ADK_PY_CHECK_EXCEPTION();
        return res;
    }

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
    operator [](const Object &key) const
    {
        return GetItem(key);
    }

    Object
    operator [](const char *key) const
    {
        return GetItem(Object(key));
    }

    void
    SetItem(const Object &key, const Object &value)
    {
        if (UNLIKELY(PyObject_SetItem(_obj, key.Get(), value.Get()) == -1)) {
            ADK_PY_CHECK_EXCEPTION();
        }
    }

    void
    DelItem(const Object &key)
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

    /** Get name of the object type. */
    std::string
    TypeName() const
    {
        Object type = Type();
        PyTypeObject *pType = reinterpret_cast<PyTypeObject *>(type.Get());
        return pType->tp_name;
    }

    /** Check if object type name is the specified one. */
    bool
    CheckType(const std::string &typeName) const
    {
        return typeName == TypeName();
    }

    /** Check if object type is the specified one. */
    bool
    CheckType(PyTypeObject *typeObj) const
    {
        return reinterpret_cast<PyTypeObject *>(Type().Get()) == typeObj;
    }

    template <class ExpClass>
    bool
    CheckType()
    {
        return CheckType(ExpClass::pTypeObject);
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
        if (UNLIKELY(!argsObj)) {
            ADK_PY_CHECK_EXCEPTION();
        }
        Object res(PyObject_CallObject(_obj, argsObj.Get()));
        if (UNLIKELY(!res)) {
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
    CallMethod(const std::string &name, const Args&... args)
    {
        Object method(GetAttr(name));
        if (UNLIKELY(!method)) {
            ADK_PY_CHECK_EXCEPTION();
        }
        return method(args...);
    }
};

/** Wrapper for Python sequence protocol API. */
class ObjectSequence: public Object {
public:
    ObjectSequence(PyObject *obj = nullptr, bool isNew = true):
        Object(obj, isNew)
    {
        if (UNLIKELY(_obj && !PySequence_Check(_obj))) {
            ADK_EXCEPTION(adk::Exception, "Not a sequence type");
        }
    }

    ObjectSequence(const Object &obj): Object(obj)
    {
        if (UNLIKELY(_obj && !PySequence_Check(_obj))) {
            ADK_EXCEPTION(adk::Exception, "Not a sequence type");
        }
    }

    ObjectSequence(Object &&obj): Object(std::move(obj))
    {
        if (UNLIKELY(_obj && !PySequence_Check(_obj))) {
            ADK_EXCEPTION(adk::Exception, "Not a sequence type");
        }
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
    ObjectUnicode(PyObject *obj = nullptr, bool isNew = true):
        ObjectSequence(obj, isNew)
    {
        if (UNLIKELY(_obj && !PyUnicode_Check(_obj))) {
            ADK_EXCEPTION(adk::Exception, "Not convertible to Unicode");
        }
    }

    ObjectUnicode(const Object &obj): ObjectSequence(obj)
    {
        if (UNLIKELY(_obj && !PyUnicode_Check(_obj))) {
            ADK_EXCEPTION(adk::Exception, "Not convertible to Unicode");
        }
    }

    ObjectUnicode(Object &&obj): ObjectSequence(std::move(obj))
    {
        if (UNLIKELY(_obj && !PyUnicode_Check(_obj))) {
            ADK_EXCEPTION(adk::Exception, "Not convertible to Unicode");
        }
    }

    ObjectUnicode(const std::string &s):
        ObjectSequence(PyUnicode_FromString(s.c_str()))
    {}

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
    ObjectDict(PyObject *obj = nullptr, bool isNew = true):
        Object(obj, isNew)
    {
        if (UNLIKELY(_obj && !PyDict_Check(_obj))) {
            ADK_EXCEPTION(adk::Exception, "Not convertible to dictionary");
        }
    }

    ObjectDict(const Object &obj): Object(obj)
    {
        if (UNLIKELY(_obj && !PyDict_Check(_obj))) {
            ADK_EXCEPTION(adk::Exception, "Not convertible to dictionary");
        }
    }

    ObjectDict(Object &&obj): Object(std::move(obj))
    {
        if (UNLIKELY(_obj && !PyDict_Check(_obj))) {
            ADK_EXCEPTION(adk::Exception, "Not convertible to dictionary");
        }
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

    ObjectSequence
    Keys()
    {
        ObjectSequence res = PyDict_Keys(_obj);
        if (UNLIKELY(!res)) {
            ADK_PY_CHECK_EXCEPTION();
        }
        return res;
    }

    ObjectSequence
    Values()
    {
        ObjectSequence res = PyDict_Values(_obj);
        if (UNLIKELY(!res)) {
            ADK_PY_CHECK_EXCEPTION();
        }
        return res;
    }

    ObjectSequence
    Items()
    {
        ObjectSequence res = PyDict_Items(_obj);
        if (UNLIKELY(!res)) {
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
    ObjectModule(PyObject *obj = nullptr, bool isNew = true): Object(obj, isNew)
    {
    }

    /** The constructor imports the module with the specified name. */
    ObjectModule(const std::string &name):
        Object(PyImport_ImportModule(name.c_str()))
    {

    }

    /** This constructor creates new module. */
    ObjectModule(PyModuleDef *def):
        Object(PyModule_Create(def))
    {

    }

    /** Add object to the module. */
    void
    AddObject(const std::string &name, const Object &obj)
    {
        if (UNLIKELY(PyModule_AddObject(_obj, name.c_str(), obj.Get(true)) == -1)) {
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
