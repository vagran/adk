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

sc.AddOption('--adk-prefix',
             dest = 'adkPrefix',
             nargs = 1,
             action  ='store',
             metavar = 'ADK_PREFIX',
             help = 'ADK installation prefix.')


def GetAdkPrefix():
    prefix = sc.GetOption('adkPrefix')
    if prefix is None:
        if 'ADK_PREFIX' in os.environ:
            prefix = os.environ['ADK_PREFIX']
        else:
            prefix = '/usr'
    return prefix


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
        'TEST_DESC': None,
        'TEST_SRCS': '',
        
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
    
        self.ADK_PREFIX = GetAdkPrefix()
    
    
    def IsDesktop(self):
        return self.PLATFORM != 'avr'
    
    
    def IsStatic(self):
        return self.APP_TYPE != 'dynamic_lib'
    
    
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
        
        self.CCFLAGS += ' -mmcu=%s -fshort-wchar ' % self.MCU
        self.DEFS += ' ADK_MCU=%s ADK_MCU_FREQ=%d' % (self.MCU, self.MCU_FREQ)
        self.ASFLAGS += ' -mmcu=%s ' % self.MCU
        
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
            else:
                result.append(file)
        return result
    
    
    def _BuildObjs(self, e, srcs):
        objs = list()
        buildDir = e.Dir('.').path
        for srcFile in srcs:
            if os.path.commonprefix([srcFile.path, buildDir]) != buildDir:
                path = os.path.join('external', srcFile.path)
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
        
        e['ADK_ROOT'] = self.ADK_ROOT
        e['ADK_PREFIX'] = self.ADK_PREFIX

        self._HandleSubdirs(e)
        
        if self.APP_NAME is None:
            return
        
        if (self.APP_TYPE != 'app' and self.APP_TYPE != 'static_lib' and
            self.APP_TYPE != 'dynamic_lib' and self.APP_TYPE != 'unit_test'):
            raise Exception('Unsupported application type: ' + self.APP_TYPE)
        
        if self.USE_GUI is None and self.IsDesktop():
            self.USE_GUI = self.APP_TYPE != 'unit_test'
        
        self.INCLUDE_DIRS += ' ${ADK_ROOT}/include '
        
        
        if self.USE_GUI:
            self.DEFS += ' ADK_USE_GUI '
        if self.USE_PYTHON:
            self.DEFS += ' ADK_USE_PYTHON '
        
        
        self._SetupCrossCompiling(e)
        
        
        if self.AVR_USE_USB:
            self.DEFS += ' ADK_AVR_USE_USB '
            if self.IsDesktop():
                self.PKGS += ' libusb '
        
        if self.APP_TYPE == 'unit_test':
            if self.TEST_DESC is None:
                raise Exception('Unit test description should be specified')
            self.DEFS += ' UNITTEST '
            self.SRC_DIRS += ' ${ADK_ROOT}/src/unit_test '
            e['BUILDERS']['UtAutoSrc'] = e.Builder(action = Conf._BuildUtAutoSrc)
        
        adkFlags = '-Wall -Werror -Wextra '
        if e.GetOption('adkBuildType') == 'debug' or self.APP_TYPE == 'unit_test':
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
        self.CXXFLAGS += ' -std=c++11 '
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
            
        self.PCHS += ' %s ' % os.path.join(self.ADK_ROOT, 'include', 'adk.h')
        
        if self.IsDesktop() and not self.NO_ADK_LIB:
            self.LIBS += ' adk '
            self.LIB_DIRS += ' %s ' % os.path.join(self.ADK_PREFIX, 'lib')
        
        # Include directories
        e['CPPPATH'] = self._ProcessFilesList(e, self.INCLUDE_DIRS, e.Dir, True)
        
        # Compiler flags
        e['CFLAGS'] = self.CFLAGS
        e['CXXFLAGS'] = self.CXXFLAGS
        e['CCFLAGS'] = self.CCFLAGS
        e['ASFLAGS'] = self.ASFLAGS
        
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
        if self.USE_GUI:
            self.PKGS += ' gtkmm-3.0 '
        if self.USE_PYTHON:
            self.PKGS += ' python3 '
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
                
        resFiles = self._ProcessFilesList(e, self.RES_FILES)
        if self.USE_GUI:
            resFiles += sc.Glob('*.glade')
            
        pchs = self._ProcessFilesList(e, self.PCHS)
        
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
        
        srcObjs = self._BuildObjs(e, srcFiles)
            
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
            
            
        if self.APP_TYPE == 'unit_test':
            testObjs = self._BuildObjs(e, self._ProcessFilesList(e, self.TEST_SRCS))
            autoSrc = e.UtAutoSrc('auto/auto_stubs.cpp',
                                  e.File('SConscript'), # Need some dependency
                                  ADK_TEST_DESC = self.TEST_DESC,
                                  ADK_TEST_OBJS = testObjs,
                                  ADK_OBJS = srcObjs,
                                  ADK_PLATFORM_ID = self.PLATFORM_ID)
            e.Depends(autoSrc, srcObjs)
            e.Depends(autoSrc, testObjs)
            srcObjs.extend(testObjs)
            srcObjs.extend(self._BuildObjs(e, autoSrc))
            

        if self.APP_TYPE == 'app' or self.APP_TYPE == 'unit_test':
            output = e.Program(self.APP_NAME, srcObjs + resAsms)
        elif self.APP_TYPE == 'dynamic_lib':
            output = e.SharedLibrary(self.APP_NAME, srcObjs + resAsms)
        else:
            output = e.StaticLibrary(self.APP_NAME, srcObjs + resAsms)
    
        if self.APP_ALIAS is not None:
            e.Alias(self.APP_ALIAS, output)
        else:
            e.Alias(self.APP_NAME, output)
        e.Default(output)
        
        if self.PLATFORM == 'avr':
            self._HandleAvrBuild(e, output)
        
        for installDir in self._ProcessFilesList(e, self.INSTALL_DIR, e.Dir):
            i = e.Install(installDir, output)
            if self.APP_ALIAS is not None:
                e.Alias(self.APP_ALIAS + '-install', i)
            e.Alias('install', i)
    
    
    def _HandleSubdirs(self, e):
        '''
        Build subprojects in the SUBDIRS attribute.
        @return SCons nodes returned from the scripts in the subdirectories.
        '''
        dirs = self._ProcessFilesList(e, self.SUBDIRS, e.Dir)
        result = list()
        
        if sc.Dir('.').path == '.':
            buildDirName = os.path.join('build', '%s-%s' % (self.PLATFORM,
                                                            sc.GetOption('adkBuildType')))
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
        e.Default(romHex)
        romSrec = e.AvrRomSrec(output[0].abspath + '_rom.srec', output)
        e.Default(romSrec)
        romBin = e.AvrRomBin(output[0].abspath + '_rom.bin', output)
        e.Default(romBin)
        eepromHex = e.AvrEepromHex(output[0].abspath + '_eeprom.hex', output)
        e.Default(eepromHex)
        eepromSrec = e.AvrEepromSrec(output[0].abspath + '_eeprom.srec', output)
        e.Default(eepromSrec)
        eepromBin = e.AvrEepromBin(output[0].abspath + '_eeprom.bin', output)
        e.Default(eepromBin)
        
        
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
        
        for libPath in env['LIBPATH']:
            opts += ' --lib-dir ' + libPath.abspath
        
#         if env['ADK_PLATFORM_ID'] == Conf.PLATFORM_ID_LINUX64:
#             opts += ' --lib-dir /usr/lib/debug/lib/x86_64-linux-gnu'
                
        a = env.Action('$ADK_ROOT/src/unit_test/ut_stubs_gen.py --result={0} {1}'.
            format(target[0].abspath, opts))
        env.Execute(a)
        with open(target[0].abspath, "a") as out:
            out.write(env.subst('''

namespace ut {

const char *__ut_test_description = "${ADK_TEST_DESC}";

} /* namespace ut */
            '''))
