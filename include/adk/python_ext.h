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

namespace internal {

/** Helper class for modules registration. */
class ModuleRegistrator {
protected:
    PyModuleDef _moduleDef = { PyModuleDef_HEAD_INIT };

    typedef PyObject* (*InitFunc)();

    void
    Register(const char *name, InitFunc initFunc);

    PyObject *
    InitModule()
    {
        return PyModule_Create(&_moduleDef);
    }

    /** Add documentation string to the module. */
    void
    Doc(const char *doc)
    {
        _moduleDef.m_doc = doc;
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
