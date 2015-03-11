# /ADK/unittest/python/test_embedding.py
#
# This file is a part of ADK library.
# Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

i = 237
f = 2.5
s = 'test string'
 
def TestFunc(a, b):
   return a + b

class TestClass:
   prop = 10

   def __init__(self, base):
       self.base = base
   def Sum(self, x):
       return self.base + x

testObj = TestClass(20)
