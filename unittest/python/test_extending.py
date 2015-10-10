# /ADK/unittest/python/test_embedding.py
#
# This file is a part of ADK library.
# Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See LICENSE file for copyright details.

import test_module
mod_help = test_module.__doc__
result = test_module.TestFuncSum(200, 37)
func_help = test_module.TestFuncSum.__doc__

class_help = test_module.TestClass.__doc__
obj = test_module.TestClass(300)
obj_hash = hash(obj)
obj_str = str(obj)
obj_repr = repr(obj)
obj_call = obj(10, 15)
meth_help = test_module.TestClass.TestMethod.__doc__
meth_call = obj.TestMethod(42)
meth2_call = obj.TestMethodNoArgs()
exception_catched = None
try:
    obj.TestMethodException()
except Exception as e:
    exception_catched = str(e)
