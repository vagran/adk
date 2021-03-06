# This file is a part of ADK library.
# Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See LICENSE file for copyright details.

import os, subprocess, platform, ConfigParser

import SCons
from posix import lstat
sc = SCons.Script

# Project-global default values for some parameters.
# Initial values which may be overwritten.
_defaultInitial = dict()
# Initial values to which user values are appended.
_defaultAppend = dict()


def DefConf(**kwargs):
    'Set project-global default initial values for some parameters.'
    _defaultInitial.update(kwargs)


def DefConfAppend(**kwargs):
    'Set project-global default initial values to which user values are appended.'
    _defaultAppend.update(kwargs)
            

def AppendValue(lst, value):
    '''
    Append the provided value to the provided list. It properly handles strings
    and lists.
    @return Concatenated values, either string or list depending on input.
    '''
    
    if isinstance(lst, list) or isinstance(value, list):
        # Handle lists
        if isinstance(lst, str):
            lst = sc.Split(lst)
        if isinstance(value, str):
            value = sc.Split(value)
        lst.extend(value)
    else:
        # Handle strings
        lst = lst + ' ' + value
    return lst


def CloneValues(lst):
    '''
    Get copy of values list.
    '''
    if isinstance(lst, str):
        # String is immutable so it is safe to return the original instance.
        return lst
    return list(lst)


sc.AddOption('--adk-build-type',
             dest = 'adkBuildType',
             nargs = 1,
             action  ='store',
             metavar = 'ADK_BUILD_TYPE',
             choices = ['debug', 'release'],
             default = None,
             help = 'Build configuration type.')

sc.AddOption('--adk-platform',
             dest = 'adkPlatform',
             nargs = 1,
             action  ='store',
             metavar = 'ADK_PLATFORM',
             choices = ['native', 'linux', 'avr'],
             help = 'Target platform.')

sc.AddOption('--adk-prefix',
             dest = 'adkPrefix',
             nargs = 1,
             action  ='store',
             metavar = 'ADK_PREFIX',
             help = 'ADK installation prefix.')

sc.AddOption('--adk-run-tests',
             dest = 'adkRunTests',
             action  ='store_true',
             default = False,
             help = 'Run unit tests.')

sc.AddOption('--adk-write-fuses',
             dest = 'adkWriteFuses',
             action  ='store_true',
             default = False,
             help = 'Write fuses to the connected MCU.')

sc.AddOption('--adk-verify-fuses',
             dest = 'adkVerifyFuses',
             action  ='store_true',
             default = False,
             help = 'Verify fuses in the connected MCU.')

sc.AddOption('--adk-upload-flash', '--adk-write-flash',
             dest = 'adkUploadFlash',
             action  ='store_true',
             default = False,
             help = 'Upload firmware flash image to the connected MCU.')

sc.AddOption('--adk-verify-flash',
             dest = 'adkVerifyFlash',
             action  ='store_true',
             default = False,
             help = 'Verify firmware flash image in the connected MCU.')

sc.AddOption('--adk-upload-eeprom', '--adk-write-eeprom',
             dest = 'adkUploadEeprom',
             action  ='store_true',
             default = False,
             help = 'Upload EEPORM image to the connected MCU.')

sc.AddOption('--adk-verify-eeprom',
             dest = 'adkVerifyEeprom',
             action  ='store_true',
             default = False,
             help = 'Verify EEPORM image in the connected MCU.')

sc.AddOption('--adk-mcu-reset',
             dest = 'adkMcuReset',
             action  ='store_true',
             default = False,
             help = 'Reset the connected MCU.')


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
        'SRC_DIRS': '.',
        'SRCS': '',
        'INCLUDE_DIRS': '',
        'CFLAGS': '',
        'CXXFLAGS': '',
        'CCFLAGS': '',
        'ASFLAGS': '',
        'DEFS': '',
        'PKGS': '',
        'RES_FILES': '',
        'PCHS': '',
        'INSTALL_DIR': '',
        'NO_ADK_LIB': False,
        'LIBS': '',
        'LIB_DIRS': '',
        'RELEASE_OPT_FLAGS': None,
        'DEBUG_OPT_FLAGS': None,
        'LINKFLAGS': '',
        'TEST_DESC': None,
        'TEST_SRCS': '',
        'JAVA_CP': None,
        'JAVA_NATIVE_CLASSES': None,
        
        'MCU': None,
        'PROGRAMMER': None,
        'PROGRAMMER_BUS': None,
        'MCU_FUSE': None,
        'MCU_LFUSE': None,
        'MCU_HFUSE': None,
        'MCU_EFUSE': None,
        'MCU_FREQ': None,
        
        'USE_GUI': None,
        'USE_PYTHON': False,
        'USE_JAVA': False,
        'AVR_USE_USB': False,
        'AVR_USE_COMMON_LIB': True
    }
    
    
    def __init__(self, **kwargs):
        
        self.isTopLevel = False
        
        for paramName in Conf.params:
            if paramName in kwargs:
                if paramName in _defaultAppend:
                    value = CloneValues(_defaultAppend[paramName])
                    value = AppendValue(value, kwargs[paramName])
                else:
                    value = kwargs[paramName]
            else:
                if paramName in _defaultInitial:
                    value = _defaultInitial[paramName]
                elif paramName in _defaultAppend:
                    value = _defaultAppend[paramName]
                else:
                    value = Conf.params[paramName]
            setattr(self, paramName, value)
            
        self.config = ConfigParser.SafeConfigParser({'home': os.path.expanduser('~')})
        confPath = os.path.expanduser('~/.adk/adk.conf')
        if os.path.exists(confPath):
            self.config.readfp(open(confPath))
    
        self.ADK_ROOT = self.GetParameter('ADK_ROOT', 'paths', None)
        if self.ADK_ROOT is None:
            if not sc.GetOption('help'):
                raise Exception('ADK_ROOT environment variable (or entry in ' +
                                'ADK configuration file) should point to ADK source')
            else:
                self.ADK_ROOT = ''
                
        self.adkBuildType = sc.GetOption('adkBuildType')
        if self.adkBuildType is None:
            if 'ADK_BUILD_TYPE' in os.environ:
                self.adkBuildType = os.environ['ADK_BUILD_TYPE']
            else:
                self.adkBuildType = 'release'
        if self.adkBuildType != 'debug' and self.adkBuildType != 'release':
            raise Exception('Invalid build type specified: ' + self.adkBuildType)
    
        cmdPlatform = sc.GetOption('adkPlatform')
        if cmdPlatform is not None:
            self.PLATFORM = cmdPlatform
    
        self.ADK_PREFIX = sc.GetOption('adkPrefix')
        if self.ADK_PREFIX is None:
            self.ADK_PREFIX = self.GetParameter('ADK_PREFIX', 'paths', '/usr')
        
        self.runTests = sc.GetOption('adkRunTests')
    
    
    def GetParameter(self, name, section, default):
        if name in os.environ:
            return os.environ[name]
        name = name.lower()
        if self.config.has_option(section, name):
            return self.config.get(section, name)
        return default


    def IsDesktop(self):
        return self.PLATFORM != 'avr'
    
    
    def IsStatic(self):
        return self.APP_TYPE != 'dynamic_lib'
    
    
    def IsLinux(self):
        return (self.PLATFORM_ID == Conf.PLATFORM_ID_LINUX32 or
                self.PLATFORM_ID == Conf.PLATFORM_ID_LINUX64)
    
    
    def _SetupNativePlatform(self, e):
        e['OBJCOPY'] = 'objcopy'
        
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
        e['CC'] = 'avr-gcc'
        e['CXX'] = 'avr-g++'
        e['OBJCOPY'] = 'avr-objcopy'
    
        if self.MCU is None:
            raise Exception('MCU must be set for AVR target')
        if self.MCU_FREQ is None:
            raise Exception('MCU clock frequency must be set for AVR target')
        
        self.CCFLAGS += ' -mmcu=%s -fshort-wchar -fshort-enums ' % self.MCU
        self.CXXFLAGS += ' -fno-exceptions -fno-rtti ';
        self.DEFS += ' ADK_MCU=%s ADK_MCU_FREQ=%d' % (self.MCU, self.MCU_FREQ)
        self.ASFLAGS += ' -mmcu=%s ' % self.MCU
        self.LINKFLAGS += ' -mmcu=%s ' % self.MCU
        
        if self.MCU_FUSE is not None:
            self.DEFS += ' ADK_MCU_FUSE=0x%x ' % self.MCU_FUSE
        if self.MCU_LFUSE is not None:
            self.DEFS += ' ADK_MCU_LFUSE=0x%x ' % self.MCU_LFUSE
        if self.MCU_HFUSE is not None:
            self.DEFS += ' ADK_MCU_HFUSE=0x%x ' % self.MCU_HFUSE
        if self.MCU_EFUSE is not None:
            self.DEFS += ' ADK_MCU_EFUSE=0x%x ' % self.MCU_EFUSE
    
        if self.RELEASE_OPT_FLAGS is None:
            self.RELEASE_OPT_FLAGS = '-Os -mcall-prologues'
        if self.DEBUG_OPT_FLAGS is None:
            self.DEBUG_OPT_FLAGS = '-Os -mcall-prologues'
        
        if self.AVR_USE_COMMON_LIB:
            self.SRC_DIRS += ' ${ADK_ROOT}/src/libavr/common '
            self.DEFS += ' ADK_AVR_USE_COMMON_LIB '
        
        if self.AVR_USE_USB:
            self.SRC_DIRS += ' ${ADK_ROOT}/src/libavr/usb '
    
    
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
    
    
    @staticmethod
    def TransformResFileName(name):
        result = ''
        for c in name:
            if ((c >= '0' and c <= '9') or (c >= 'a' and c <= 'z') or
                (c >= 'A' and c <= 'Z')):
                result += c
            else:
                result += '_'
        return result
        
    
    @staticmethod
    def _BuildResAsm(target, source, env):
        resName = Conf.TransformResFileName(os.path.basename(source[0].path))
        with open(target[0].abspath, "w") as out:
            out.write('''
/* This file is automatically generated. */
    .section .rodata
    .global _binary_{0}_start
    .type _binary_{0}_start, @object
    .global _binary_{0}_end
    .type _binary_{0}_end, @object
    .align 4
_binary_{0}_start:
    .incbin "{1}"
_binary_{0}_end:
            '''.format(resName, source[0].abspath))
    
    
    @staticmethod
    def _BuildResHdr(target, source, env):
        resName = Conf.TransformResFileName(os.path.basename(source[0].path))
        with open(target[0].abspath, "w") as out:
            out.write('''
/* This file is automatically generated. */
extern "C" const char _binary_{0}_start;
extern "C" const char _binary_{0}_end;
ADK_DECL_RESOURCE({0}, "{1}", \\
                  &_binary_{0}_start, \\
                  &_binary_{0}_end)
            '''.format(resName, os.path.basename(source[0].path)))
    
    
    @staticmethod
    def _BuildResIndexHdr(target, source, env):
        with open(target[0].abspath, "w") as f:
            f.write('/* This file is generated automatically. */\n\n')
            for src in source:
                f.write("#include <%s>\n" % os.path.basename(src.path))
                    
            
    def _CreateResBuilder(self, e):
        'Create builders for embedding raw resource files.'
        
        e['BUILDERS']['ResAsm'] = e.Builder(action = Conf._BuildResAsm)
        e['BUILDERS']['ResHdrFile'] = e.Builder(action = Conf._BuildResHdr)
        e['BUILDERS']['ResIndexHdrFile'] = e.Builder(action = Conf._BuildResIndexHdr)
    
    
    @staticmethod
    def _BuildJavaHdr(target, source, env):
        a = env.Action('{} -cp {} -o {} {}'
            .format(env['ADK_JAVAH'], 
                    ':'.join(map(lambda cp: cp.abspath, env['ADK_JAVA_CP'])),
                    target[0].abspath,
                    ' '.join(env['ADK_JAVA_NATIVE_CLASSES'])))
        env.Execute(a)
    
        
    def _CreateJavaBuilder(self, e):
        'Create builders for JNI support'
        
        e['BUILDERS']['JavaHdr'] = e.Builder(action = Conf._BuildJavaHdr)
        
    
    @staticmethod
    def _BuildGch(target, source, env):
        a = env.Action('$CXX -o {0} -x c++-header -c $CXXFLAGS $CCFLAGS $_CCCOMCOM {1}'.
            format(target[0].abspath, source[0].abspath))
        env.Execute(a)
            
            
    @staticmethod      
    def _BuildGchShared(target, source, env):
        a = env.Action('$CXX -o {0} -x c++-header -c $SHCXXFLAGS $SHCCFLAGS $_CCCOMCOM {1}'.
            format(target[0].abspath, source[0].abspath))
        env.Execute(a)
    
    
    def _CreatePchBuilder(self, e):
        'Create builders for pre-compiled headers'
        
        e['BUILDERS']['Gch'] = e.Builder(action = Conf._BuildGch)
        e['BUILDERS']['GchShared'] = e.Builder(action = Conf._BuildGchShared)
        
    
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
    
    
    def _ProcessFilesList(self, e, files, factory = None, isSrc = False):
        if factory is None:
            factory = e.File
        if isinstance(files, str):
            files = sc.Split(e.subst(files))
        result = list()
        for file in files:
            if isinstance(file, str):
                node = factory(e.subst(file))
                if isSrc:
                    node = node.srcnode()
                result.append(node)
            elif isinstance(file, list):
                result.extend(self._ProcessFilesList(e, file, factory, isSrc))
            else:
                result.append(file)
        return result
    
    
    def _BuildObjs(self, e, srcs):
        objs = list()
        buildDir = e.Dir('.').path
        for srcFile in srcs:
            if os.path.commonprefix([srcFile.path, buildDir]) != buildDir:
                srcPath = srcFile.path
                if srcPath[0] == '/':
                    srcPath = srcPath[1:]
                path = os.path.join('external', srcPath)
                objPath = self._GetObjPath(e, path)
            else:
                objPath = self._GetObjPath(e, srcFile.path[len(buildDir) + 1:])
            if self.IsStatic():
                obj = e.StaticObject(objPath, srcFile)
            else:
                obj = e.SharedObject(objPath, srcFile)
            objs.append(obj)
        return objs
    
    
    def Build(self):
        '''
        Apply build configuration.
        @return: SCons node for the result binary. None if nothing built.
        '''
        
        e = sc.Environment()
        
        self._CreateResBuilder(e)
        self._CreatePchBuilder(e)
        self._CreateJavaBuilder(e)
        
        e['ADK_ROOT'] = self.ADK_ROOT
        e['ADK_PREFIX'] = self.ADK_PREFIX
        e['VALGRIND'] = ('valgrind -q --suppressions=${ADK_ROOT}/tools/valgrind.supp ' +
                         '--error-exitcode=255 --leak-check=full --gen-suppressions=all')

        self._HandleSubdirs(e)
        
        if self.APP_NAME is None:
            return
        
        if (self.APP_TYPE != 'app' and self.APP_TYPE != 'static_lib' and
            self.APP_TYPE != 'dynamic_lib' and self.APP_TYPE != 'unit_test'):
            raise Exception('Unsupported application type: ' + self.APP_TYPE)
        
        if self.USE_GUI is None and self.IsDesktop():
            self.USE_GUI = self.APP_TYPE == 'app'
        
        self.INCLUDE_DIRS += ' ${ADK_ROOT}/include '
        
        self._SetupCrossCompiling(e)  
        
        if self.USE_GUI:
            self.DEFS += ' ADK_USE_GUI '
        if self.USE_PYTHON:
            self.DEFS += ' ADK_USE_PYTHON '
        if self.USE_JAVA:
            self.DEFS += ' ADK_USE_JAVA '
            javaInc = os.path.join(self.USE_JAVA, 'include')
            if self.IsLinux():
                javaPlatformInc = os.path.join(javaInc, 'linux')
            else:
                raise Exception('Unsupported platform for JNI')
            self.INCLUDE_DIRS += ' ' + javaInc + ' ' + javaPlatformInc + ' '
        
        if self.AVR_USE_USB:
            self.DEFS += ' ADK_AVR_USE_USB '
            if self.IsDesktop():
                self.PKGS += ' libusb-1.0 '
        
        if self.APP_TYPE == 'unit_test':
            if self.TEST_DESC is None:
                raise Exception('Unit test description should be specified')
            self.DEFS += ' UNITTEST '
            self.SRC_DIRS += ' ${ADK_ROOT}/src/unit_test '
            e['BUILDERS']['UtAutoSrc'] = e.Builder(action = Conf._BuildUtAutoSrc)
        
        adkFlags = '-Wall -Werror -Wextra -Wno-terminate'
        if self.adkBuildType == 'debug' or self.APP_TYPE == 'unit_test':
            adkFlags += ' -ggdb3 '
            self.DEFS += ' DEBUG '
            if self.DEBUG_OPT_FLAGS is None:
                adkFlags += ' -O0 '
            else:
                adkFlags += ' %s ' % self.DEBUG_OPT_FLAGS
        else:
            if self.RELEASE_OPT_FLAGS is None:
                adkFlags += ' -O2 '
            else:
                adkFlags += ' %s ' % self.RELEASE_OPT_FLAGS
        
        self.CCFLAGS = adkFlags + self.CCFLAGS
        self.CXXFLAGS += ' -std=c++1y '
        self.CFLAGS += ' -std=c99 '
        
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
            
        if len(self.PCHS) == 0:
            self.PCHS = ' %s ' % os.path.join(self.ADK_ROOT, 'include', 'adk.h')
        
        if self.IsDesktop() and not self.NO_ADK_LIB:
            self.LIBS += ' adk '
            self.LIB_DIRS += ' %s ' % os.path.join(self.ADK_PREFIX, 'lib')
            
        if self.IsDesktop():
            self.CCFLAGS += ' -pthread '
        
        # Include directories
        e['CPPPATH'] = self._ProcessFilesList(e, self.INCLUDE_DIRS, e.Dir, True)
        
        # Compiler flags
        e['CFLAGS'] = self.CFLAGS
        e['CXXFLAGS'] = self.CXXFLAGS
        e['CCFLAGS'] = self.CCFLAGS
        e['ASFLAGS'] = self.ASFLAGS
        e['LINKFLAGS'] = self.LINKFLAGS
        
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
            self.PKGS += ' glibmm-2.4 giomm-2.4 expat '
        if self.USE_GUI:
            self.PKGS += ' gtkmm-3.0 '
        if self.USE_PYTHON:
            self.PKGS += ' python-3.5 '
        self._SetupPackages(e)
        
        # Libraries
        e.Append(LIBS = sc.Split(self.LIBS))
        e.Append(LIBPATH = self._ProcessFilesList(e, self.LIB_DIRS, e.Dir))
        
        srcFiles = list()
        for srcDir in self._ProcessFilesList(e, self.SRC_DIRS, e.Dir):
            srcFiles.extend(srcDir.glob('*.c'))
            srcFiles.extend(srcDir.glob('*.cpp'))
            srcFiles.extend(srcDir.glob('*.s'))
            srcFiles.extend(srcDir.glob('*.S'))
        for src in self._ProcessFilesList(e, self.SRCS):
            srcFiles.append(src)
            
        if self.USE_JAVA:
            srcFiles.extend(e.Dir(os.path.join(self.ADK_ROOT, 'src', 'lib', 'java')).glob('*.cpp'))
                
        resFiles = self._ProcessFilesList(e, self.RES_FILES)
        if self.USE_GUI:
            resFiles += sc.Glob('*.glade')
            
        pchs = self._ProcessFilesList(e, self.PCHS)
        
        # Embedded resources
        resHdrs = list()
        resAsms = list()
        hdrsDir = 'auto_include_' + self.APP_TYPE
        for resFile in resFiles:
            fn = os.path.join(hdrsDir, 
                              os.path.basename(resFile.path) + '.res.s')
            resAsms.extend(e.ResAsm(fn, resFile))
            resHdrs.extend(e.ResHdrFile(os.path.join(hdrsDir, 
                                                     os.path.basename(resFile.path) + '.res.h'), 
                                        resFile))
        resIndexHdr = e.ResIndexHdrFile(os.path.join(hdrsDir, 'auto_adk_res.h'), 
                                        resHdrs)
        e.Prepend(CPPPATH = e.Dir(hdrsDir).abspath)
        
        # JNI header
        if self.USE_JAVA:
            jniHdr = e.JavaHdr(os.path.join(hdrsDir, 'auto_adk_jni.h'),
                               None,
                               ADK_JAVAH = os.path.join(self.USE_JAVA, 'bin', 'javah'),
                               ADK_JAVA_CP = self._ProcessFilesList(e, self.JAVA_CP, e.Dir),
                               ADK_JAVA_NATIVE_CLASSES = self._ProcessFilesList(e, self.JAVA_NATIVE_CLASSES, str))
        
        srcObjs = self._BuildObjs(e, srcFiles + resAsms)
            
        # Precompiled headers
        gchs = list()
        for pch in pchs:
            fn = os.path.join(hdrsDir, os.path.basename(pch.path) + '.gch')
            if self.IsStatic():
                gch = e.Gch(fn, pch)
            else:
                gch = e.GchShared(fn, pch)
            e.Depends(gch, resIndexHdr)
            if self.USE_JAVA:
                e.Depends(gch, jniHdr)
            gchs.append(gch)
        
        for obj in srcObjs:
            e.Depends(obj, resIndexHdr)
            e.Depends(obj, gchs)
            if self.USE_JAVA:
                e.Depends(obj, jniHdr)
            
        # Auto-stubs for unit tests
        if self.APP_TYPE == 'unit_test':
            testObjs = self._BuildObjs(e, self._ProcessFilesList(e, self.TEST_SRCS))
            autoSrc = e.UtAutoSrc('auto/auto_stubs.cpp',
                                  srcObjs + testObjs,
                                  ADK_TEST_DESC = self.TEST_DESC,
                                  ADK_TEST_OBJS = testObjs,
                                  ADK_OBJS = srcObjs,
                                  ADK_PLATFORM_ID = self.PLATFORM_ID,
                                  ADK_LIBS = 'c stdc++')
            srcObjs.extend(testObjs)
            srcObjs.extend(self._BuildObjs(e, autoSrc))
            

        if self.APP_TYPE == 'app' or self.APP_TYPE == 'unit_test':
            output = e.Program(self.APP_NAME, srcObjs)
        elif self.APP_TYPE == 'dynamic_lib':
            output = e.SharedLibrary(self.APP_NAME, srcObjs)
        else:
            output = e.StaticLibrary(self.APP_NAME, srcObjs)
    
        if self.APP_ALIAS is not None:
            e.Alias(self.APP_ALIAS, output)
        else:
            e.Alias(self.APP_NAME, output)
        if not self.runTests:
            e.Default(output)
        
        if self.PLATFORM == 'avr':
            self._HandleAvrBuild(e, output)
        
        for installDir in self._ProcessFilesList(e, self.INSTALL_DIR, e.Dir):
            i = e.Install(installDir, output)
            if self.APP_ALIAS is not None:
                e.Alias(self.APP_ALIAS + '-install', i)
            e.Alias('install', i)
            
        if self.runTests and self.APP_TYPE == 'unit_test':
            libPath = ':'.join(map(str, e['LIBPATH']))
            if len(libPath) > 0:
                libPath = 'env LD_LIBRARY_PATH=' + libPath
            cmd = e.Command('TestResult_' + self.APP_NAME, output,
                            '%s $VALGRIND $SOURCE' % libPath)
            e.AlwaysBuild(cmd)
            e.Default(cmd)
            
        return output
    
    
    def _HandleSubdirs(self, e):
        '''
        Build subprojects in the SUBDIRS attribute.
        @return SCons nodes returned from the scripts in the subdirectories.
        '''
        dirs = self._ProcessFilesList(e, self.SUBDIRS, e.Dir)
        result = list()
        
        # Build prefix for top-level directory
        if sc.Dir('.').abspath == sc.Dir('#').abspath:
            buildDirName = os.path.join('build', '%s-%s' % (self.PLATFORM,
                                                            self.adkBuildType))
        else:
            buildDirName = ''
        
        for dir in dirs:
            res = sc.SConscript(os.path.join(dir.abspath, 'SConscript'),
                                variant_dir = buildDirName,
                                duplicate = False,
                                src_dir = '.')
            if res is None:
                continue
            if isinstance(res, list):
                result.extend(res)
            else:
                result.append(res)
                 
        return result

    
    def _HandleAvrBuild(self, e, output):
        'Handle additional actions for AVR build'
        
        # Binary converted to text formats understandable by most firmware uploaders.
        e['BUILDERS']['AvrRomHex'] = e.Builder(action = Conf._BuildAvrRomHex)
        e['BUILDERS']['AvrRomSrec'] = e.Builder(action = Conf._BuildAvrRomSrec)
        e['BUILDERS']['AvrRomBin'] = e.Builder(action = Conf._BuildAvrRomBin)
        e['BUILDERS']['AvrEepromHex'] = e.Builder(action = Conf._BuildAvrEepromHex)
        e['BUILDERS']['AvrEepromSrec'] = e.Builder(action = Conf._BuildAvrEepromSrec)
        e['BUILDERS']['AvrEepromBin'] = e.Builder(action = Conf._BuildAvrEepromBin)
        
        romHex = e.AvrRomHex(output[0].abspath + '_rom.hex', output)
        romSrec = e.AvrRomSrec(output[0].abspath + '_rom.srec', output)
        romBin = e.AvrRomBin(output[0].abspath + '_rom.bin', output)
        eepromHex = e.AvrEepromHex(output[0].abspath + '_eeprom.hex', output)
        eepromSrec = e.AvrEepromSrec(output[0].abspath + '_eeprom.srec', output)
        eepromBin = e.AvrEepromBin(output[0].abspath + '_eeprom.bin', output)
        
        if not self.runTests:
            
            def PrintSize(target, source, env):
                a = env.Action('@size %s' % source[0].path)
                print('\n=========================== Image size ===========================')
                env.Execute(a)
                print('==================================================================\n')
                
            cmd = e.Command('IMG_SIZE', output, PrintSize)
            e.Default(cmd)
            
            e.Default(romHex)
            e.Default(romSrec)
            e.Default(romBin)
            e.Default(eepromHex)
            e.Default(eepromSrec)
            e.Default(eepromBin)
        
        avrdude = 'avrdude -p {} -c {} -P {}'.format(self.MCU, self.PROGRAMMER,
                                                     self.PROGRAMMER_BUS)
        
        if e.GetOption('adkWriteFuses'):
            if self.MCU_FUSE is not None:
                cmd = e.Command('W_FUSE', None,
                                '%s -U fuse:w:0x%x:m' % (avrdude, self.MCU_FUSE))
                e.AlwaysBuild(cmd)
                e.Default(cmd)
            if self.MCU_LFUSE is not None:
                cmd = e.Command('W_LFUSE', None,
                                '%s -U lfuse:w:0x%x:m' % (avrdude, self.MCU_LFUSE))
                e.AlwaysBuild(cmd)
                e.Default(cmd)
            if self.MCU_HFUSE is not None:
                cmd = e.Command('W_HFUSE', None,
                                '%s -U hfuse:w:0x%x:m' % (avrdude, self.MCU_HFUSE))
                e.AlwaysBuild(cmd)
                e.Default(cmd)
            if self.MCU_EFUSE is not None:
                cmd = e.Command('W_EFUSE', None,
                                '%s -U efuse:w:0x%x:m' % (avrdude, self.MCU_EFUSE))
                e.AlwaysBuild(cmd)
                e.Default(cmd)
                
        if e.GetOption('adkVerifyFuses'):
            if self.MCU_FUSE is not None:
                cmd = e.Command('V_FUSE', None,
                                '%s -U fuse:v:0x%x:m' % (avrdude, self.MCU_FUSE))
                e.AlwaysBuild(cmd)
                e.Default(cmd)
            if self.MCU_LFUSE is not None:
                cmd = e.Command('V_LFUSE', None,
                                '%s -U lfuse:v:0x%x:m' % (avrdude, self.MCU_LFUSE))
                e.AlwaysBuild(cmd)
                e.Default(cmd)
            if self.MCU_HFUSE is not None:
                cmd = e.Command('V_HFUSE', None,
                                '%s -U hfuse:v:0x%x:m' % (avrdude, self.MCU_HFUSE))
                e.AlwaysBuild(cmd)
                e.Default(cmd)
            if self.MCU_EFUSE is not None:
                cmd = e.Command('V_EFUSE', None,
                                '%s -U efuse:v:0x%x:m' % (avrdude, self.MCU_EFUSE))
                e.AlwaysBuild(cmd)
                e.Default(cmd)
                
        if e.GetOption('adkUploadFlash'):
            cmd = e.Command('U_FLASH', None,
                            '%s -U flash:w:%s:i' % (avrdude, romHex[0].path))
            e.Depends(cmd, romHex)
            e.AlwaysBuild(cmd)
            e.Default(cmd)
            
        if e.GetOption('adkVerifyFlash'):
            cmd = e.Command('V_FLASH', None,
                            '%s -U flash:v:%s:i' % (avrdude, romHex[0].path))
            e.Depends(cmd, romHex)
            e.AlwaysBuild(cmd)
            e.Default(cmd)
            
        if e.GetOption('adkUploadEeprom'):
            cmd = e.Command('U_EEPROM', None,
                            '%s -U eeprom:w:%s:i' % (avrdude, eepromHex[0].path))
            e.Depends(cmd, eepromHex)
            e.AlwaysBuild(cmd)
            e.Default(cmd)
            
        if e.GetOption('adkVerifyEeprom'):
            cmd = e.Command('U_EEPROM', None,
                            '%s -U eeprom:v:%s:i' % (avrdude, eepromHex[0].path))
            e.Depends(cmd, eepromHex)
            e.AlwaysBuild(cmd)
            e.Default(cmd)
            
        if e.GetOption('adkMcuReset'):
            cmd = e.Command('RESET', None, avrdude)
            e.AlwaysBuild(cmd)
            e.Default(cmd)
        
    @staticmethod
    def _BuildAvrRomHex(target, source, env):
        a = env.Action('$OBJCOPY -j .text -j .data -O ihex {0} {1}'.
            format(source[0].abspath, target[0].abspath))
        env.Execute(a)
        
        
    @staticmethod
    def _BuildAvrRomSrec(target, source, env):
        a = env.Action('$OBJCOPY -j .text -j .data -O srec {0} {1}'.
            format(source[0].abspath, target[0].abspath))
        env.Execute(a)
        
        
    @staticmethod
    def _BuildAvrRomBin(target, source, env):
        a = env.Action('$OBJCOPY -j .text -j .data -O binary {0} {1}'.
            format(source[0].abspath, target[0].abspath))
        env.Execute(a)
    
    
    @staticmethod
    def _BuildAvrEepromHex(target, source, env):
        a = env.Action('$OBJCOPY -j .eeprom --change-section-lma .eeprom=0 -O ihex {0} {1}'.
            format(source[0].abspath, target[0].abspath))
        env.Execute(a)
        
        
    @staticmethod
    def _BuildAvrEepromSrec(target, source, env):
        a = env.Action('$OBJCOPY -j .eeprom --change-section-lma .eeprom=0 -O srec {0} {1}'.
            format(source[0].abspath, target[0].abspath))
        env.Execute(a)
        
        
    @staticmethod
    def _BuildAvrEepromBin(target, source, env):
        a = env.Action('$OBJCOPY -j .eeprom --change-section-lma .eeprom=0 -O binary {0} {1}'.
            format(source[0].abspath, target[0].abspath))
        env.Execute(a)

    @staticmethod
    def _BuildUtAutoSrc(target, source, env):
        opts = ''
        
        for srcList in env['ADK_OBJS']:
            for src in srcList:
                opts += ' --src ' + src.path
                
        for srcList in env['ADK_TEST_OBJS']:
            for src in srcList:
                opts += ' --test-src ' + src.path
                
        for lib in env['LIBS']:
            opts += ' --lib ' + lib
        for lib in sc.Split(env['ADK_LIBS']):
            opts += ' --lib ' + lib
        opts += ' --lib pthread'
        for libPath in env['LIBPATH']:
            opts += ' --lib-dir ' + libPath.abspath
                
        a = env.Action('$ADK_ROOT/src/unit_test/ut_stubs_gen.py --result={0} {1}'.
            format(target[0].abspath, opts))
        env.Execute(a)
        with open(target[0].abspath, "a") as out:
            out.write(env.subst('''

namespace ut {

const char *__ut_test_description = "${ADK_TEST_DESC}";

} /* namespace ut */
            '''))
