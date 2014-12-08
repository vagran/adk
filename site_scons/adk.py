# This file is a part of ADK library.
# Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

import os, subprocess, platform

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
        'RES_FILES': '',
        
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
    
    
    def IsStatic(self):
        return self.APP_TYPE != 'dynamic_lib'
    
    
    def _SetupNativePlatform(self, e):
        arch = platform.architecture()
        mach = platform.machine()
        
        if arch[0] == '32bit':
            self.WORD_SIZE = 32
        elif arch[0] == '64bit':
            self.WORD_SIZE = 64
        else:
            raise Exception('Unsupported host word size: ' + arch[0])
        
        if mach == 'i386' or mach == 'x86_64':
            e['OBJ_ARCH'] = 'i386'
        else:
            raise Exception('Unsupported host machine type: ' + mach)
        
        if arch[1] == 'ELF':
            e['OBJ_FORMAT'] = 'elf%d-%s' % (self.WORD_SIZE, mach)
        else:
            raise Exception('Unsupported host linkage type: ' + arch[1])
            
    
    def _SetupCrossCompiling(self, e):
        e['OBJCOPY'] = 'objcopy'
        if self.PLATFORM == 'native':
            self._SetupNativePlatform(e)
            return
        if self.PLATFORM == 'avr':
            self.WORD_SIZE = 16
            self._SetupAvrCompiling(e)
        else:
            raise Exception('Unsupported cross compilation target platform: ' +
                            self.PLATFORM)
    
    
    def _SetupAvrCompiling(self, e):
        #XXX
        raise Exception('not implemented')
    
    
    def _SetupPackage(self, pkg, e):
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
        
    
    def _SetupPackages(self, e):
        pkgs = sc.Split(self.PKGS)
        for pkg in pkgs:
            self._SetupPackage(pkg, e)
            
    
    def _CreateResBuilder(self, e):
        'Create for embedding raw resource files.'
        
        def BuildResStaticObj(target, source, env):
            print('BuildResStaticObj called "%s": "%s"' % (repr(target), repr(source)))
            
        e['BUILDERS']['ResStaticObjFile'] = e.Builder(action = BuildResStaticObj)
            
            
        def BuildResDynamicObj(target, source, env):
            print('BuildResDynamicObj called "%s": "%s"' % (repr(target), repr(source)))
        
        e['BUILDERS']['ResDynamicObjFile'] = e.Builder(action = BuildResDynamicObj)
        
        
        def BuildResHdr(target, source, env):
            print('BuildResHdr called "%s": "%s"' % (repr(target), repr(source)))
        
        e['BUILDERS']['ResHdrFile'] = e.Builder(action = BuildResHdr)
        
        
        def BuildResIndexHdr(target, source, env):
            print('writing ' + target[0].abspath)
            with open(target[0].abspath, "w") as f:
                f.write('/* This file is generated automatically. */\n')
        
        e['BUILDERS']['ResIndexHdrFile'] = e.Builder(action = BuildResIndexHdr)
        
        
    def _GetObjPath(self, e, path, isStatic = None):
        if isStatic is None:
            isStatic = self.IsStatic()
        dirName = os.path.dirname(path)
        baseName = os.path.basename(path)
        if isStatic:
            baseName = e.subst('${OBJPREFIX}%s${OBJSUFFIX}' % baseName)
        else:
            baseName = e.subst('${SHOBJPREFIX}%s${SHOBJSUFFIX}' % baseName)
        return os.path.join(dirName, baseName)
    
    
    def _ProcessFilesList(self, e, files):
        if isinstance(files, str):
            files = sc.Split(files)
        result = list()
        for file in files:
            if isinstance(file, str):
                result.append(e.File(file))
            else:
                result.append(file)
        return result
    
    
    def Build(self):
        '''
        Apply build configuration.
        @return: SCons node for the result binary. None if nothing built.
        '''
        
        e = sc.Environment()
        
        self._CreateResBuilder(e)
        
        self._HandleSubdirs()
        
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
        
        if self.USE_GUI:
            self.DEFS += ' ADK_USE_GUI '
        if self.USE_PYTHON:
            self.DEFS += ' ADK_USE_PYTHON '
        
        self._SetupCrossCompiling(e)
        
        
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
            self.PKGS += ' gtkmm-3.0 python3 '
        self._SetupPackages(e)
        
        
        cFiles = sc.Glob('*.c')
        cppFiles = sc.Glob('*.cpp')
        asmFiles = sc.Glob('*.s')
    
        resFiles = self._ProcessFilesList(e, self.RES_FILES)
        if self.USE_GUI:
            resFiles += sc.Glob('*.glade')
        
        srcFiles = cFiles + cppFiles + asmFiles
        
        resObjs = list()
        resHdrs = list()
        hdrsDir = 'auto_include_' + self.APP_TYPE
        for resFile in resFiles:
            fn = self._GetObjPath(e, os.path.basename(resFile.path))
            print(fn)
            if self.IsStatic():
                resObjs.append(e.ResStaticObjFile(fn, resFile))
            else:
                resObjs.append(e.ResDynamicObjFile(fn, resFile))
            resHdrs.append(e.ResHdrFile(os.path.join(hdrsDir, 
                                                     os.path.basename(resFile.path) + '.h'), 
                                        resFile))
        resIndexHdr = e.ResIndexHdrFile(os.path.join(hdrsDir, 'auto_adk_res.h'), 
                                        resHdrs)
        e.Append(CPPPATH = e.Dir(hdrsDir).abspath)
        
        if self.IsStatic():
            srcObjs = [e.StaticObject(f) for f in srcFiles]
        elif self.APP_TYPE == 'dynamic_lib':
            srcObjs = [e.SharedObject(f) for f in srcFiles]
        else:
            raise Exception('Unsupported application type: ' + self.APP_TYPE)
        
        for obj in srcObjs:
            e.Depends(obj, resIndexHdr)

        if self.APP_TYPE == 'app':
            output = e.Program(self.APP_NAME, srcObjs + resObjs)
        elif self.APP_TYPE == 'dynamic_lib':
            output = e.SharedLibrary(self.APP_NAME, srcObjs + resObjs)
        else:
            output = e.StaticLibrary(self.APP_NAME, srcObjs + resObjs)
        
        if self.APP_ALIAS is not None:
            e.Alias(self.APP_ALIAS, output)
        e.Default(output)
        
        
    def _HandleSubdirs(self):
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
