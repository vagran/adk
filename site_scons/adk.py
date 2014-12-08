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
    
    (PLATFORM_ID_AVR,
     PLATFORM_ID_LINUX32,
     PLATFORM_ID_LINUX64,
     PLATFORM_ID_WIN32,
     PLATFORM_ID_WIN64) = range(5)
    
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
        'PCHS': '',
        
        'USE_GUI': None,
        'USE_PYTHON': False,
        'AVR_USE_USB': False
    }
    
    
    def __init__(self, **kwargs):
        
        self.isTopLevel = False
        
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
        sys = platform.system()
        
        if arch[0] == '32bit':
            self.WORD_SIZE = 32
        elif arch[0] == '64bit':
            self.WORD_SIZE = 64
        else:
            raise Exception('Unsupported host word size: ' + arch[0])
        
        if sys == 'Linux':
            if self.WORD_SIZE == 32:
                self.PLATFORM_ID = Conf.PLATFORM_ID_LINUX32
            else:
                self.PLATFORM_ID = Conf.PLATFORM_ID_LINUX64
        else:
            raise Exception('Unsupported native system')
        
#         if mach == 'i386' or mach == 'x86_64':
#             e['OBJ_ARCH'] = 'i386'
#         else:
#             raise Exception('Unsupported host machine type: ' + mach)
#         
#         if arch[1] == 'ELF':
#             e['OBJ_FORMAT'] = 'elf%d-%s' % (self.WORD_SIZE, mach)
#         else:
#             raise Exception('Unsupported host linkage type: ' + arch[1])
            
    
    def _SetupCrossCompiling(self, e):
        if self.PLATFORM == 'native':
            self._SetupNativePlatform(e)
            return
        if self.PLATFORM == 'avr':
            self.WORD_SIZE = 16
            self.PLATFORM_ID = Conf.PLATFORM_ID_AVR
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
        'Create builders for embedding raw resource files.'
        
        def TransformResFileName(name):
            result = ''
            for c in name:
                if ((c >= '0' and c <= '9') or (c >= 'a' and c <= 'z') or
                    (c >= 'A' and c <= 'Z')):
                    result += c
                else:
                    result += '_'
            return result
        
        def BuildResAsm(target, source, env):
            resName = TransformResFileName(os.path.basename(source[0].path))
            with open(target[0].abspath, "w") as out:
                out.write('''
/* This file is automatically generated. */
    .section rodata
    .global _binary_{0}_start
    .type _binary_{0}_start, @object
    .global _binary_{0}_end
    .type _binary_{0}_end, @object
    .align 4
_binary_{0}_start:
    .incbin "{1}"
_binary_{0}_end:
                '''.format(resName, source[0].abspath))
            
        e['BUILDERS']['ResAsm'] = e.Builder(action = BuildResAsm)
            
        
        def BuildResHdr(target, source, env):
            resName = TransformResFileName(os.path.basename(source[0].path))
            with open(target[0].abspath, "w") as out:
                out.write('''
/* This file is automatically generated. */
extern "C" const char _binary_{0}_start;
extern "C" const char _binary_{0}_end;
ADK_DECL_RESOURCE({0}, "{1}", \\
                  &_binary_{0}_start, \\
                  &_binary_{0}_end)
                '''.format(resName, os.path.basename(source[0].path)))
        
        e['BUILDERS']['ResHdrFile'] = e.Builder(action = BuildResHdr)
        
        
        def BuildResIndexHdr(target, source, env):
            with open(target[0].abspath, "w") as f:
                f.write('/* This file is generated automatically. */\n\n')
                for src in source:
                    f.write("#include <%s>\n" % os.path.basename(src.path))
        
        e['BUILDERS']['ResIndexHdrFile'] = e.Builder(action = BuildResIndexHdr)
        
    
    def _CreatePchBuilder(self, e):
        'Create builders for pre-compiled headers'
        
        def BuildGch(target, source, env):
            a = e.Action('$CXX -o {0} -x c++-header -c $CXXFLAGS $CCFLAGS $_CCCOMCOM {1}'.
                format(target[0].abspath, source[0].abspath))
            e.Execute(a)
        
        e['BUILDERS']['Gch'] = e.Builder(action = BuildGch)
        
        def BuildGchShared(target, source, env):
            a = e.Action('$CXX -o {0} -x c++-header -c $SHCXXFLAGS $SHCCFLAGS $_CCCOMCOM {1}'.
                format(target[0].abspath, source[0].abspath))
            e.Execute(a)
        
        e['BUILDERS']['GchShared'] = e.Builder(action = BuildGchShared)
        
    
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
        self._CreatePchBuilder(e)

        self._HandleSubdirs()
        
        if self.APP_NAME is None:
            return
        
        if (self.APP_TYPE != 'app' and self.APP_TYPE != 'static_lib' and
            self.APP_TYPE != 'dynamic_lib'):
            raise Exception('Unsupported application type: ' + self.APP_TYPE)
        
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
        
        self.DEFS += ' ADK_PLATFORM_ID=%d ' % self.PLATFORM_ID
        if self.PLATFORM_ID == Conf.PLATFORM_ID_AVR:
            self.DEFS += ' ADK_PLATFORM_AVR '
        elif self.PLATFORM_ID == Conf.PLATFORM_ID_LINUX32:
            self.DEFS += ' ADK_PLATFORM_LINUX32 '
        elif self.PLATFORM_ID == Conf.PLATFORM_ID_LINUX64:
            self.DEFS += ' ADK_PLATFORM_LINUX64 '
        elif self.PLATFORM_ID == Conf.PLATFORM_ID_WINDOWS32:
            self.DEFS += ' ADK_PLATFORM_WINDOWS32 '
        elif self.PLATFORM_ID == Conf.PLATFORM_ID_WINDOWS64:
            self.DEFS += ' ADK_PLATFORM_WINDOWS64 '
            
        self.PCHS += ' %s ' % os.path.join(self.ADK_ROOT, 'include', 'adk.h')
        
        
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
            
        pchs = self._ProcessFilesList(e, self.PCHS)
        
        srcFiles = cFiles + cppFiles + asmFiles
        
        resHdrs = list()
        resAsms = list()
        hdrsDir = 'auto_include_' + self.APP_TYPE
        for resFile in resFiles:
            fn = os.path.join(hdrsDir, 
                              os.path.basename(resFile.path) + '.res.s')
            resAsms.append(e.ResAsm(fn, resFile))
            resHdrs.append(e.ResHdrFile(os.path.join(hdrsDir, 
                                                     os.path.basename(resFile.path) + '.res.h'), 
                                        resFile))
        resIndexHdr = e.ResIndexHdrFile(os.path.join(hdrsDir, 'auto_adk_res.h'), 
                                        resHdrs)
        e.Prepend(CPPPATH = e.Dir(hdrsDir).abspath)
        
        
        if self.IsStatic():
            srcObjs = [e.StaticObject(f) for f in srcFiles]
        else:
            srcObjs = [e.SharedObject(f) for f in srcFiles]
        
        gchs = list()
        for pch in pchs:
            fn = os.path.join(hdrsDir, os.path.basename(pch.path) + '.gch')
            if self.IsStatic():
                gch = e.Gch(fn, pch)
            else:
                gch = e.GchShared(fn, pch)
            e.Depends(gch, resIndexHdr)
            gchs.append(gch)
            
        for obj in srcObjs:
            e.Depends(obj, resIndexHdr)
            e.Depends(obj, gchs)

        if self.APP_TYPE == 'app':
            output = e.Program(self.APP_NAME, srcObjs + resAsms)
        elif self.APP_TYPE == 'dynamic_lib':
            output = e.SharedLibrary(self.APP_NAME, srcObjs + resAsms)
        else:
            output = e.StaticLibrary(self.APP_NAME, srcObjs + resAsms)
    
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
        
        if sc.Dir('.').path == '.':
            buildDirName = os.path.join('build', '%s-%s' % (self.PLATFORM,
                                                            sc.GetOption('adkBuildType')))
        else:
            buildDirName = ''
        
        for dir in dirs:
            res = sc.SConscript(os.path.join(dir, 'SConscript'),
                                variant_dir = buildDirName,
                                duplicate = False,
                                src_dir = '.')
            if res is None:
                continue
            if isinstance(res, list):
                result.extend(res)
            else:
                result.append(res)
