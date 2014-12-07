# This file is a part of ADK library.
# Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

import os, subprocess

import SCons
sc = SCons.Script


sc.AddOption('--adk-build-type',
             dest = 'adkBuildType',
             nargs = 1,
             action  ='store',
             metavar = 'ADK_BUILD_TYPE',
             choices = ['debug', 'release'],
             default = 'release',
             help = 'Build configuration type.')
sc.AddOption('--adk-platform',
             dest = 'adkPlatform',
             nargs = 1,
             action  ='store',
             metavar = 'ADK_PLATFORM',
             choices = ['native', 'linux', 'avr'],
             help = 'Target platform.')


class Conf(object):
    
    # Possible attributes for configuration and their default values.
    params = {
        'PLATFORM': 'native',
        'APP_TYPE': 'app',
        'APP_NAME': None,
        'APP_ALIAS': None,
        'SUBDIRS': '',
        'INCLUDE_DIRS': '',
        'CFLAGS': '',
        'CXXFLAGS': '',
        'CCFLAGS': '',
        'DEFS': '',
        'PKGS': '',
        
        'USE_GUI': None,
        'USE_PYTHON': False,
        'AVR_USE_USB': False
    }
    
    
    def __init__(self, **kwargs):
        
        for paramName in Conf.params:
            if paramName in kwargs:
                value = kwargs[paramName]
            else:
                value = Conf.params[paramName]
            setattr(self, paramName, value)
    
        if 'ADK_ROOT' in os.environ:
            self.ADK_ROOT = os.environ['ADK_ROOT']
        else:
            if not sc.GetOption('help'):
                raise Exception('ADK_ROOT environment variable should point to ADK source')
            else:
                self.ADK_ROOT = ''
    
        cmdPlatform = sc.GetOption('adkPlatform')
        if cmdPlatform is not None:
            self.PLATFORM = cmdPlatform
    
    
    def IsDesktop(self):
        return self.PLATFORM != 'avr'
    
    
    def SetupCrossCompiling(self, e):
        if self.PLATFORM == 'native':
            return
        if self.PLATFORM == 'avr':
            self.SetupAvrCompiling(e)
        else:
            raise Exception('Unsupported cross compilation target platform: ' +
                            self.PLATFORM)
    
    
    def SetupAvrCompiling(self, e):
        #XXX
        raise Exception('not implemented')
    
    
    def SetupPackage(self, pkg, e):
        if '=' in pkg:
            pkgName, version = pkg.split('=')
            opt = '--exact-version=' + version
        elif '>' in pkg:
            pkgName, version = pkg.split('>')
            opt = '--atleast-version=' + version
        elif '<' in pkg:
            pkgName, version = pkg.split('<')
            opt = '--max-version=' + version
        else:
            pkgName = pkg
            opt = '--exists'
        
        if subprocess.call(['pkg-config', opt, pkgName]) != 0:
            raise Exception('Package not found: %s\nPlease install development files for this package' % pkg)
        
        e.ParseConfig('pkg-config %s --cflags --libs' % pkgName)
        
    
    def SetupPackages(self, e):
        pkgs = sc.Split(self.PKGS)
        for pkg in pkgs:
            self.SetupPackage(pkg, e)
    
    
    def Build(self):
        '''
        Apply build configuration.
        @return: SCons node for the result binary. None if nothing built.
        '''
        
        e = sc.Environment()
        
        self.HandleSubdirs()
        
        if self.APP_NAME is None:
            return
        
        if self.USE_GUI is None and self.IsDesktop():
            self.USE_GUI = True
        
        self.INCLUDE_DIRS += ' ' + os.path.join(self.ADK_ROOT, 'include')
        
        adkFlags = '-Wall -Werror -Wextra '
        if e.GetOption('adkBuildType') == 'debug':
            adkFlags += ' -ggdb3 -O0 '
            self.DEFS += ' DEBUG'
        else:
            adkFlags += '-O2'
        
        self.CCFLAGS = adkFlags + self.CCFLAGS
        self.CXXFLAGS += ' -std=c++11'
        self.CFLAGS += ' -std=c99'
        
        self.SetupCrossCompiling(e)
        
        
        # Include directories
        e['CPPPATH'] = sc.Split(self.INCLUDE_DIRS)
        
        # Compiler flags
        e['CFLAGS'] = self.CFLAGS
        e['CXXFLAGS'] = self.CXXFLAGS
        e['CCFLAGS'] = self.CCFLAGS
        
        # Preprocessor macros
        defs = sc.Split(self.DEFS)
        DEFS = list()
        for item in defs:
            if '=' in item:
                DEFS.append(tuple(item.split('=')))
            else:
                DEFS.append(item)
        e['CPPDEFINES'] = DEFS
        
        
        if self.IsDesktop():
            self.PKGS += ' glibmm-2.4 giomm-2.4 '
        self.SetupPackages(e)
        
        
        cFiles = sc.Glob('*.c')
        cppFiles = sc.Glob('*.cpp')
        asmFiles = sc.Glob('*.s')
        
        srcFiles = cFiles + cppFiles + asmFiles
        
        if self.APP_TYPE == 'app':
            output = e.Program(self.APP_NAME, srcFiles)
        elif self.APP_TYPE == 'dynamic_lib':
            output = e.SharedLibrary(self.APP_NAME, srcFiles)
        elif self.APP_TYPE == 'static_lib':
            output = e.StaticLibrary(self.APP_NAME, srcFiles)
        else:
            raise Exception('Unsupported application type: ' + self.APP_TYPE)
        
        if self.APP_ALIAS is not None:
            e.Alias(self.APP_ALIAS, output)
        e.Default(output)
        
        
    def HandleSubdirs(self):
        '''
        Build subprojects in the SUBDIRS attribute.
        @return SCons nodes returned from the scripts in the subdirectories.
        '''
        dirs = sc.Split(self.SUBDIRS)
        result = list()
        for dir in dirs:
            res = sc.SConscript(os.path.join(dir, 'SConscript'),
                                variant_dir = 'build', duplicate = 0)
            if res is None:
                continue
            if isinstance(res, list):
                result.extend(res)
            else:
                result.append(res)
