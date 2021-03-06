#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /ADK/src/unit_test/ut_stubs_gen.pl
#
# This file is a part of ADK library.
# Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See LICENSE file for copyright details.

# This tool automatically generates stubs for all unreferenced symbols in order
# satisfy unit tests linking.

import os
import sys
from optparse import OptionParser
import re
import subprocess
import shutil
import shlex
import glob

usage = '''
Stubs generator.

Usage:
%prog [options]
'''

# All globally defined symbols
defined_syms = dict()
# All undefined symbols
wanted_syms = dict()
# Prohibited symbols
prohibited_syms = {'__dso_handle': 1}
# List of libraries fetched from ldconfig
ldconfigCache = list()

def Error(msg, exception = None):
    print('ERROR: ' + msg);
    print('===================================================================')
    if exception is None:
        raise Exception(msg);
    else:
        raise

class ExecOutput(list):
    pass

def Exec(cmd, errorFatal = True):
    '''
    Execute shell command and return list of lines from its standard output.
    Raise an exception if the command return code is not zero.
    '''
    output = ExecOutput()
    p = subprocess.Popen(shlex.split(cmd), stdout = subprocess.PIPE,
                         stderr = subprocess.STDOUT)
    while True:
        line = p.stdout.readline().decode('utf-8')
        if len(line) == 0 and p.poll() != None:
            break
        if len(line) != 0:
            line = line.rstrip()
            output.append(line)
    if p.returncode != 0:
        if errorFatal:
            Error('Shell command \'%s\' returned status %d' % (cmd, p.returncode))
    output.exitcode = p.returncode
    return output

def ParseFile(filename, isTest, isDynamicLib):
    global opts, defined_syms, wanted_syms

    if isDynamicLib:
        cmd = '{} -D {}'.format(opts.nm, filename)
    else:
        cmd = '{} {}'.format(opts.nm, filename)
    output = Exec(cmd, errorFatal = False)
    if (output.exitcode != 0 or 
        (len(output) > 0 and re.match('.*\\Wno symbols$', output[0]) is not None)):
        
        # Try opposite command. Detached debug symbols for libraries are not
        # dynamic.
        if isDynamicLib:
            cmd = '{} {}'.format(opts.nm, filename)
        else:
            cmd = '{} -D {}'.format(opts.nm, filename)
        output = Exec(cmd)
    
    globalTypes = 'TDRBWViu'
    pat = re.compile('^\\s*[0-9a-fA-F]*\\s+([{}U])\\s+(.*)$'.format((globalTypes)))
    for line in output:
        m = pat.match(line)
        if m is None:
            continue
        name = m.group(2)
        if m.group(1) in globalTypes:
            if '@' in name:
                name = name[:name.index('@')]
            defined_syms[name] = filename
        elif m.group(1) == 'U' and isTest:
            if (not name.startswith('__cxa') and
                not name.startswith('__cxx') and
                not name.startswith('__gxx') and
                not name.startswith('_Unwind_')):
                
                wanted_syms[name] = filename

def FetchLdconfig():
    'Fetch paths from ldconfig'
    output = Exec('/sbin/ldconfig -p', errorFatal = False)
    if output.exitcode != 0:
        return
    pat = re.compile('^.*=>\\s+(\\S+)')
    for line in output:
        m = pat.match(line)
        if m is None:
            continue
        ldconfigCache.append(m.group(1))

def FindLib(libname):
    global opts
    
    for path in ldconfigCache:
        bn = os.path.basename(path)
        idx = bn.find('.so')
        if idx == -1:
            return
        if bn[:idx] == 'lib' + libname:
            yield path
    
    for dir in opts.lib_dirs:
        file = '{}/lib{}.so'.format(dir, libname)
        if os.path.exists(file):
            yield file
    
    for dir in opts.lib_dirs:
        for file in glob.iglob('{}/lib{}.so.*'.format(dir, libname)):
            yield file
        for file in glob.iglob('{}/lib{}-*.so'.format(dir, libname)):
            yield file

def ProcessLib(libname):
    found = False
    for file in FindLib(libname):
        try:
            ParseFile(file, False, True)
            # Check also debug symbols directory
            if file.startswith('/usr'):
                file = file[4:]
            path = os.path.join('/usr/lib/debug', os.path.dirname(file.lstrip('/')))
            for dbgFile in glob.iglob('{}/lib{}.so.*'.format(path, libname)):
                ParseFile(dbgFile, False, False)
            for dbgFile in glob.iglob('{}/lib{}-*.so'.format(path, libname)):
                ParseFile(dbgFile, False, False)
            found = True
        except:
            continue
        break
    if not found:
        Error('Library not found: "{}"'.format(libname))
    
def ResolveSymbols():
    delList = list()
    for name in wanted_syms:
        if name in defined_syms or name in prohibited_syms:
            delList.append(name)
    for name in delList:
        del wanted_syms[name] 

def DemangleName(name):
    output = Exec('{} {}'.format(opts.cppfilt, name))
    return output[0].strip()
    
def CreateOutput():
    f = open(opts.result, 'w')
    header = '''
/* This file is automatically generated by ADK unit testing framework.
 * Do not edit it manually!
 */

#include <adk_ut.h>

#define SYM_STUB(idx, name, readable_name, wanted_by) \\
    namespace ut { \\
    void ut_auto_stub_ ## idx() asm(name); \\
    void ut_auto_stub_ ## idx() { \\
        UT_FAIL("Auto-generated stub for '" readable_name "' called\\n" \\
                "(referenced from '" wanted_by "')"); \\
    } \\
    } /* namespace ut */

'''
    f.write(header)
    
    symIdx = 0
    for name in wanted_syms:
        readable_name = DemangleName(name)
        f.write('SYM_STUB({},\n\t"{}",\n\t"{}",\n\t"{}")\n'.format(
                    symIdx, name, readable_name, wanted_syms[name]));
        symIdx += 1
    f.close()
    print('Stubs generated for {} symbols'.format(symIdx))
    
def Main():
    global opts

    optParser = OptionParser(usage = usage)
    optParser.add_option('--nm', dest = 'nm',
                         metavar = 'NM',
                         help = 'nm utility path.',
                         type = 'str',
                         default = 'nm')
    optParser.add_option('--cppfilt', dest = 'cppfilt',
                         metavar = 'CPPFILT',
                         help = 'c++filt utility path.',
                         type = 'str',
                         default = 'c++filt')
    optParser.add_option('--result', dest = 'result',
                         metavar = 'CPPFILT',
                         help = 'c++filt utility path.',
                         type = 'str')
    optParser.add_option('--src', dest = 'srcs',
                         metavar = 'SRC_FILE',
                         help = 'Source files of the test',
                         type = 'str',
                         action = 'append')
    optParser.add_option('--test-src', dest = 'test_srcs',
                         metavar = 'TEST_SRC_FILE',
                         help = 'Source files being tested',
                         type = 'str',
                         action = 'append')
    optParser.add_option('--lib-dir', dest = 'lib_dirs',
                         metavar = 'LIB_DIR',
                         help = 'Directories to search for libraries',
                         type = 'str',
                         action = 'append')
    optParser.add_option('--lib', dest = 'libs',
                         metavar = 'LIB',
                         help = 'Libraries to resolve symbols from',
                         type = 'str',
                         action = 'append')
    
    (opts, args) = optParser.parse_args()
    
    if opts.result is None:
        Error('Result file not specified')
    
    if opts.srcs is None:
        Error('Source files of the test not specified')
        
    FetchLdconfig()
    
    if opts.test_srcs is not None:
        for file in opts.test_srcs:
            ParseFile(file, True, False)
    
    for file in opts.srcs:
        ParseFile(file, False, False)
    
    for lib in opts.libs:
        ProcessLib(lib)
    
    ResolveSymbols()
    
    CreateOutput()
    
if __name__ == "__main__":
    Main()
