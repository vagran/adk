/* /ADK/include/adk/python_ext.h
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file python_ext.h
 * Declarations for Python extending API (extending embedded Python
 * functionality by implementation in C++).
 */

#ifndef ADK_PYTHON_EXT_H_
#define ADK_PYTHON_EXT_H_

namespace adk {

namespace py {

/** Base class for all user classes exposed from C++ to Python. */
class ExposedClassBase {
private:
    struct {
        PyObject_HEAD
    } obj;
public:
    virtual
    ~ExposedClassBase()
    {
    }

    /** Get underlying Python object pointer. */
    PyObject *
    GetObject()
    {
        return &obj.ob_base;
    }

    /** Get pointer to this class object by its underlying Python object pointer. */
    template <class Cls = ExposedClassBase>
    static Cls *
    GetClassObject(PyObject *pObj)
    {
        /* offsetof will produce compilation error */
        size_t offset =
            reinterpret_cast<uintptr_t>(&reinterpret_cast<Cls *>(static_cast<uintptr_t>(1))->obj) - 1;
        return static_cast<Cls *>(reinterpret_cast<ExposedClassBase *>
            (reinterpret_cast<u8 *>(pObj) - offset));
    }

    /* Built-in method default implementation. Actually they are not exposed
     * unless user class overrides them.
     */

    /** Exposed as Python __init__ method. */
    int
    Init(Object args, Object kwArgs)
    {
        return 0;
    }

    /** Exposed as Python __repr__ method. */
    Object
    Repr()
    {
        return Object();
    }

    /** Exposed as Python __str__ method. */
    Object
    Str()
    {
        return Object();
    }

    /** Exposed as Python __hash__ method. */
    Py_hash_t
    Hash()
    {
        return Object();
    }

    /** Calling operator automatically exposed to Python as '__call__' method. */
    Object
    operator()(Object args, Object kwArgs)
    {
        return Object();
    }

    /** Print object to file. */
    int
    Print(FILE *fp, int flags)
    {
        return 0;
    }

    Object
    GetAttr(Object attrName)
    {
        return Object();
    }

    int
    SetAttr(Object attrName, Object attrValue)
    {
        return 0;
    }
};

namespace internal {

/** Helper class for modules registration. */
class ModuleRegistrator {
private:
    std::vector<PyMethodDef> _methods;

    typedef Object (*TNoArgsFunc)(Object);
    typedef Object (*TVarArgsFunc)(Object, Object);
    typedef Object (*TKwArgsFunc)(Object, Object, Object);

    /** Update methods table with a new method. */
    void
    _AddMethod(const char *name, PyCFunction func, int flags, const char *doc);

    template <TNoArgsFunc func>
    static PyObject *
    _MethodWrapper(PyObject *self)
    {
        return func(Object(self, false)).Steal();
    }

    template <TVarArgsFunc func>
    static PyObject *
    _MethodWrapper(PyObject *self, PyObject *args)
    {
        return func(Object(self, false), Object(args, false)).Steal();
    }

    template <TKwArgsFunc func>
    static PyObject *
    _MethodWrapper(PyObject *self, PyObject *args, PyObject *kwArgs)
    {
        return func(Object(self, false), Object(args, false),
                    Object(kwArgs, false)).Steal();
    }

protected:
    class ClassRegistratorBase {
    protected:
        ModuleRegistrator &_modReg;
        PyTypeObject _typeObj = { PyVarObject_HEAD_INIT(nullptr, 0) };

        static PyObject *
        _Alloc(PyTypeObject *type, Py_ssize_t nItems)
        {
            ExposedClassBase *clsObj =
                static_cast<ExposedClassBase *>
                (PyMem_Malloc(adk::RoundUp2(type->tp_basicsize + type->tp_itemsize * nItems, sizeof(void *))));
            if (UNLIKELY(!clsObj)) {
                return PyErr_NoMemory();
            }
            PyObject *obj = clsObj->GetObject();
            obj->ob_refcnt = 1;
            obj->ob_type = type;
            if (nItems) {
                reinterpret_cast<PyVarObject *>(obj)->ob_size = nItems;
            }
            return obj;
        }

        static void
        _Free(void *ptr)
        {
            ExposedClassBase *clsObj =
                ExposedClassBase::GetClassObject(static_cast<PyObject *>(ptr));
            PyMem_Free(clsObj);
        }
    public:
        ClassRegistratorBase(ModuleRegistrator &modReg,
                             Py_ssize_t size, const char *name,
                             const char *doc = nullptr): _modReg(modReg)
        {
            _typeObj.tp_name = name;
            _typeObj.tp_doc = doc;
            _typeObj.tp_basicsize = size;
            _typeObj.tp_alloc = _Alloc;
            _typeObj.tp_free = _Free;
            _typeObj.tp_flags = Py_TPFLAGS_DEFAULT;
        }

        virtual ~ClassRegistratorBase() {}

        void
        AddToModule(ObjectModule &module)
        {
            if (PyType_Ready(&_typeObj) < 0) {
                ADK_EXCEPTION(adk::Exception, "PyType_Ready() failed");
            }
            module.AddObject(_typeObj.tp_name, Object(&_typeObj.ob_base.ob_base, false));
        }
    };
private:
    /* Registered classes. */
    std::list<std::unique_ptr<ClassRegistratorBase>> _classes;

protected:
    template <class Cls>
    class ClassRegistrator: public ClassRegistratorBase {
    private:
        /* Wrappers for built-in methods. */

        /** Wrapper for __init__ method. */
        static int
        _Init(PyObject *self, PyObject *args, PyObject *kwArgs)
        {
            Cls *pCls = ExposedClassBase::GetClassObject<Cls>(self);
            return pCls->Init(Object(args, false), Object(kwArgs, false));
        }

        /** Wrapper for __repr__ method. */
        static PyObject *
        _Repr(PyObject *self)
        {
            Cls *pCls = ExposedClassBase::GetClassObject<Cls>(self);
            return pCls->Repr().Steal();
        }

        /** Wrapper for __str__ method. */
        static PyObject *
        _Str(PyObject *self)
        {
            Cls *pCls = ExposedClassBase::GetClassObject<Cls>(self);
            return pCls->Str().Steal();
        }

        /** Wrapper for __hash__ method. */
        static Py_hash_t
        _Hash(PyObject *self)
        {
            Cls *pCls = ExposedClassBase::GetClassObject<Cls>(self);
            return pCls->Hash();
        }

        /** Wrapper for __call__ method. */
        static PyObject *
        _Call(PyObject *self, PyObject *args, PyObject *kwArgs)
        {
            Cls *pCls = ExposedClassBase::GetClassObject<Cls>(self);
            return ((*pCls)(Object(args, false), Object(kwArgs, false))).Steal();
        }

        static PyObject *
        _New(PyTypeObject *type, PyObject *args, PyObject *kwArgs)
        {
            PyObject *self = type->tp_alloc(type, 0);
            if (UNLIKELY(!self)) {
                return nullptr;
            }
            /* Call constructor. */
            Cls *pCls = ExposedClassBase::GetClassObject<Cls>(self);
            new (pCls) Cls(Object(args, false), Object(kwArgs, false));
            return self;
        }

        static void
        _Dealloc(PyObject *ptr)
        {
            /* Call destructor and free memory. */
            Cls *pCls = ExposedClassBase::GetClassObject<Cls>(ptr);
            pCls->~Cls();
            Py_TYPE(ptr)->tp_free(ptr);
        }

        static int
        _Print(PyObject *self, FILE *fp, int flags)
        {
            Cls *pCls = ExposedClassBase::GetClassObject<Cls>(self);
            return pCls->Print(fp, flags);
        }

        static PyObject *
        _GetAttr(PyObject *self, PyObject *attrName)
        {
            Cls *pCls = ExposedClassBase::GetClassObject<Cls>(self);
            return pCls->GetAttr(Object(attrName, false)).Steal();
        }

        static int
        _SetAttr(PyObject *self, PyObject *attrName, PyObject *attrValue)
        {
            Cls *pCls = ExposedClassBase::GetClassObject<Cls>(self);
            return pCls->SetAttr(Object(attrName, false), Object(attrValue, false));
        }

        /** Set all built-in methods for a class. */
        void
        _SetBuiltinMethods()
        {
            _typeObj.tp_new = _New;
            _typeObj.tp_dealloc = _Dealloc;
            /* Check which built-in methods are defined. */
            if (&Cls::Init != &ExposedClassBase::Init) {
                _typeObj.tp_init = _Init;
            }
            if (&Cls::Repr != &ExposedClassBase::Repr) {
                _typeObj.tp_repr = _Repr;
            }
            if (&Cls::Str != &ExposedClassBase::Str) {
                _typeObj.tp_str = _Str;
            }
            if (&Cls::Hash != &ExposedClassBase::Hash) {
                _typeObj.tp_hash = _Hash;
            }
            if (&Cls::operator() != &ExposedClassBase::operator()) {
                _typeObj.tp_call = _Call;
            }
            if (&Cls::Print != &ExposedClassBase::Print) {
                _typeObj.tp_print = _Print;
            }
            if (&Cls::GetAttr != &ExposedClassBase::GetAttr) {
                _typeObj.tp_getattro = _GetAttr;
            }
            if (&Cls::SetAttr != &ExposedClassBase::SetAttr) {
                _typeObj.tp_setattro = _SetAttr;
            }
        }

        typedef Object (Cls::*T_NoArgsMethod)();
        typedef Object (Cls::*T_VarArgsMethod)(Object);
        typedef Object (Cls::*T_KwArgsMethod)(Object, Object);

        std::vector <PyMethodDef> _methods;

        template <T_NoArgsMethod method>
        static PyObject *
        _MethodWrapper(PyObject *self)
        {
            return (Cls::template GetClassObject<Cls>(self)->*method)().Steal();
        }

        template <T_VarArgsMethod method>
        static PyObject *
        _MethodWrapper(PyObject *self, PyObject *args)
        {
            return (Cls::template GetClassObject<Cls>(self)->*method)
                (Object(args, false)).Steal();
        }

        template <T_KwArgsMethod method>
        static PyObject *
        _MethodWrapper(PyObject *self, PyObject *args, PyObject *kwArgs)
        {
            return (Cls::template GetClassObject<Cls>(self)->*method)
                (Object(args, false), Object(kwArgs, false)).Steal();
        }

        /** Add new exposed method to the class. */
        void
        _AddMethod(const char *name, PyCFunction func, int flags, const char *doc)
        {
            _methods.resize(_methods.size() + 1);
            PyMethodDef *def = &_methods.back() - 1;
            def->ml_name = name;
            def->ml_meth = func;
            def->ml_flags = flags;
            def->ml_doc = doc;
            _typeObj.tp_methods = &_methods.front();
        }
    public:
        ClassRegistrator(ModuleRegistrator &modReg, const char *name,
                         const char *doc = nullptr):
            ClassRegistratorBase(modReg, sizeof(Cls), name, doc),
            _methods(1)
        {
            _SetBuiltinMethods();
        }

        /** Expose class method.
         *
         * @param method Pointer to class method.
         * @param name Exposed method name.
         * @param doc Optional documentation string.
         * @return Reference to registrator class in order to allow chaining.
         */
        template <T_NoArgsMethod method>
        ClassRegistrator &
        DefMethod(const char *name, const char *doc = nullptr)
        {
            _AddMethod(name, reinterpret_cast<PyCFunction>(_MethodWrapper<method>),
                       METH_NOARGS, doc);
            return *this;
        }

        template <T_VarArgsMethod method>
        ClassRegistrator &
        DefMethod(const char *name, const char *doc = nullptr)
        {
            _AddMethod(name, reinterpret_cast<PyCFunction>(_MethodWrapper<method>),
                       METH_VARARGS, doc);
            return *this;
        }

        template <T_KwArgsMethod method>
        ClassRegistrator &
        DefMethod(const char *name, const char *doc = nullptr)
        {
            _AddMethod(name, reinterpret_cast<PyCFunction>(_MethodWrapper<method>),
                       METH_KEYWORDS, doc);
            return *this;
        }
    };

    ModuleRegistrator(): _methods(1)
    {}

    PyModuleDef _moduleDef = { PyModuleDef_HEAD_INIT };

    typedef PyObject* (*InitFunc)();

    void
    Register(const char *name, InitFunc initFunc);

    PyObject *
    InitModule()
    {
        ObjectModule module(&_moduleDef);
        /* Register classes. */
        for (auto &cls: _classes) {
            cls->AddToModule(module);
        }
        return module.Steal();
    }

    /** Add documentation string to the module. */
    void
    Doc(const char *doc)
    {
        _moduleDef.m_doc = doc;
    }

    /** Define Python function in a module.
     *
     * @param name Name of function exposed in Python.
     * @param func Function address. Depending on function prototype
     *      corresponding calling convention will be declared.
     * @param doc Optional documentation string.
     */
    template <TNoArgsFunc func>
    void
    DefFunc(const char *name, const char *doc = nullptr)
    {
        _AddMethod(name, reinterpret_cast<PyCFunction>(_MethodWrapper<func>),
                   METH_NOARGS, doc);
    }

    template <TVarArgsFunc func>
    void
    DefFunc(const char *name, const char *doc = nullptr)
    {
        _AddMethod(name, reinterpret_cast<PyCFunction>(_MethodWrapper<func>),
                   METH_VARARGS, doc);
    }

    template <TKwArgsFunc func>
    void
    DefFunc(const char *name, const char *doc = nullptr)
    {
        _AddMethod(name, reinterpret_cast<PyCFunction>(_MethodWrapper<func>),
                   METH_KEYWORDS, doc);
    }

    template <class Cls>
    ClassRegistrator<Cls> &
    DefClass(const char *name, const char *doc = nullptr)
    {
        ClassRegistrator<Cls> *pReg = new ClassRegistrator<Cls>(*this, name, doc);
        _classes.push_back(std::unique_ptr<ClassRegistratorBase>(pReg));
        return *pReg;
    }
};

#define __ADK_PYTHON_MODULE_REG_CLASS(moduleName) \
    __CONCAT(__adk_py_mod_reg_cls_, __CONCAT(moduleName, __LINE__))
#define __ADK_PYTHON_MODULE_REG_OBJ(moduleName) \
    __CONCAT(__adk_py_mod_reg_obj_, __CONCAT(moduleName, __LINE__))

#define ADK_PYTHON_MODULE(moduleName) \
    class __ADK_PYTHON_MODULE_REG_CLASS(moduleName): \
        public adk::py::internal::ModuleRegistrator { \
    private: \
        /* Module building method. */ \
        void Build(); \
        static __ADK_PYTHON_MODULE_REG_CLASS(moduleName) *_regObj; \
        static PyObject *Init() { \
            return _regObj->InitModule(); \
        } \
    public: \
        /* Constructor */ \
        __ADK_PYTHON_MODULE_REG_CLASS(moduleName)() \
        { \
            _regObj  = this; \
            Build(); \
            Register(__STR(moduleName), &__ADK_PYTHON_MODULE_REG_CLASS(moduleName)::Init); \
        } \
    }; \
    __ADK_PYTHON_MODULE_REG_CLASS(moduleName) *__ADK_PYTHON_MODULE_REG_CLASS(moduleName)::_regObj; \
    static __ADK_PYTHON_MODULE_REG_CLASS(moduleName) __ADK_PYTHON_MODULE_REG_OBJ(moduleName); \
    /* Module building method definition follows. */ \
    void __ADK_PYTHON_MODULE_REG_CLASS(moduleName)::Build()

} /* namespace internal */

} /* namespace py */

} /* namespace adk */

#endif /* ADK_PYTHON_EXT_H_ */