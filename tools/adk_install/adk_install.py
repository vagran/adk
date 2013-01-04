#!/usr/bin/env python3
# ADK environment building and installation utility

from optparse import OptionParser
import subprocess
import shlex
import os
import re

import conf

usage = '''
%prog [options]
'''

def Error(msg, exception = None):
    print('ERROR: ' + msg);
    print('===================================================================')
    if exception is None:
        raise Exception(msg);
    else:
        raise

def RunCmd(cmd):
    print('Executing command: ' + cmd)
    output = list()
    p = subprocess.Popen(shlex.split(cmd), stdout = subprocess.PIPE,
                         stderr = subprocess.STDOUT)
    while True:
        line = p.stdout.readline().decode()
        if line == '' and p.poll() != None:
            break
        if line != '':
            line = line.rstrip()
            print(line)
            output.append(line)
    if p.returncode != 0:
        Error('Shell command \'%s\' returned status %d' % (cmd, p.returncode))

def FindPackage(pkgName, files):
    escPkgName = pkgName.replace('+', '\\+').replace('-', '\\-').replace('.', '\\.')
    pat = re.compile('{0}\\-((?:\\d+\\.)*\\d+)\\.(.*)'.format(escPkgName))
    for file in files:
        m = pat.match(file)
        if m is not None:
            return {'name': pkgName, 'filename': file, 'version': m.group(1),
                    'extension': m.group(2)}
    Error('Package not found: ' + pkgName)

def FindPackages(pkgList):
    files = os.listdir(opts.pkgDir)
    installPkgs = dict()
    for pkg in pkgList:
        pkgName = pkg.split(':')[0]
        if pkgName in installPkgs:
            continue
        installPkgs[pkgName] = FindPackage(pkgName, files)
    return installPkgs

def GetExtractFlag(ext):
    '''Get flag for tar for extracting archive with specified extension.'''
    if ext in ('tgz', 'tar.gz'):
        return '-z'
    if ext in ('tbz2', 'tar.bz2'):
        return '-j'
    if ext in ('txz', 'tar.xz'):
        return '-J'
    if ext in ('tar.lzma',):
        return '--lzma'
    Error('Unrecognized archive extension: ' + ext)
    
def CreateBuildEnv(pkg, target, srcDir, objDir):
    '''Create dictionary with build environment variables which can be used in
    build options.
    '''
    env = dict()
    env['PREFIX'] = opts.prefix
    env['TARGET'] = target
    env['PKG_NAME'] = pkg['name']
    env['PKG_VERSION'] = pkg['version']
    env['SRC_DIR'] = srcDir
    env['OBJ_DIR'] = objDir
    return env

def ExpandBuildParams(params, env):
    pat = re.compile('\\${(.*?)}')
    
    pos = 0
    while True:
        m = pat.search(params, pos)
        if m is None:
            break
        if m.group(1) not in env:
            Error('Build variable not found: ' + m.group(1))
        val = env[m.group(1)]
        pos = m.start() + len(val)
        params = params[0 : m.start()] + val + params[m.end() :]
    return params

def BuildPackage(pkg, target = 'default'):
    print('Building package "{0}:{1}"...'.format(pkg['name'], target))
    pkgDir = opts.buildDir + '/' + pkg['name'] + '-' + pkg['version']
    if not os.path.isdir(pkgDir):
        os.mkdir(pkgDir)
    srcDir = pkgDir + '/src'
    if not os.path.isdir(srcDir):
        # Extract package and rename directory
        xFlag = GetExtractFlag(pkg['extension'])
        RunCmd('tar -xv {0} -C {1} -f {2}'.format(xFlag, pkgDir, opts.pkgDir + '/' + pkg['filename']))
        os.rename(pkgDir + '/' + pkg['name'] + '-' + pkg['version'], srcDir)

    buildOpts = None
    if pkg['name'] in conf.opts:
        if target in conf.opts[pkg['name']]:
            buildOpts = conf.opts[pkg['name']][target]
    
    # Create working directory
    if buildOpts is None or 'no-obj-dir' not in buildOpts or not buildOpts['no-obj-dir']:
        objDir = pkgDir + '/obj-' + target
        if not os.path.isdir(objDir):
            os.mkdir(objDir)
    else:
        objDir = srcDir
    os.chdir(objDir)
    
    env = CreateBuildEnv(pkg, target, srcDir, objDir)
    
    # Configure the package
    confArgs = ['--prefix ' + opts.prefix]
    if buildOpts is not None:
        if 'config-params' in buildOpts:
            confArgs.append(ExpandBuildParams(buildOpts['config-params'], env))
    RunCmd(srcDir + '/configure ' + ' '.join(confArgs))
    
    # Build the package
    if opts.jobs is None:
        cmd = 'make'
    else:
        cmd = 'make -j {}'.format(opts.jobs)
    RunCmd(cmd)
    
    # Install the package
    RunCmd('make install')
    
    if buildOpts is not None and 'post-install-cmd' in buildOpts:
        RunCmd(ExpandBuildParams(buildOpts['post-install-cmd'], env))
    
    print('Finished building package "{0}:{1}"'.format(pkg['name'], target))

def NormalizePath(path):
    return os.path.normpath(os.path.realpath(path))

def Main():
    global opts
    
    optParser = OptionParser(usage = usage)
    
    optParser.add_option('-p', '--prefix', dest = 'prefix', default = 'local',
                         metavar = 'PREFIX-DIR',
                         help = 'Destination directory for this installation')
    optParser.add_option('-d', '--package-dir', dest = 'pkgDir', default = '.',
                         metavar = 'DIR',  
                         help = 'Directory with source code packages')
    optParser.add_option('-b', '--build-dir', dest = 'buildDir', default = 'build',
                         metavar = 'DIR',  
                         help = 'Directory where packages will be built')
    optParser.add_option('-i', '--install-pkg', dest = 'installPkgs', action = 'append',
                         metavar = 'PACKAGE',
                         help = 'Separate packages to install (multiple can be specified)')
    optParser.add_option('-e', '--exclude-pkg', dest = 'excludePkgs', action = 'append',
                         metavar = 'PACKAGE',
                         help = 'Separate packages to exclude (multiple can be specified)')
    
    optParser.add_option('-j', '--jobs', dest = 'jobs',
                         metavar = 'NUM_JOBS', type = 'int',
                         help = 'Number of jobs to run compilation by (number of CPUs should be the best choice)')
    
    (opts, args) = optParser.parse_args()
    
    opts.pkgDir = NormalizePath(opts.pkgDir)
    opts.buildDir = NormalizePath(opts.buildDir)
    opts.prefix = NormalizePath(opts.prefix)
    
    # Normalize packages in the list - add default target if none is specified
    for pkg in conf.packages:
        if not ':' in pkg:
            conf.packages[conf.packages.index(pkg)] = pkg + ':default'
    
    if opts.installPkgs is not None:
        pkgList = opts.installPkgs
        for pkg in pkgList:
            if not ':' in pkg:
                pkgList[pkgList.index(pkg)] = pkg + ':default'
        pkgList.sort(key = lambda k: conf.packages.index(k))
    else:
        pkgList = conf.packages
    
    if opts.excludePkgs is not None:
        excludeList = opts.excludePkgs
        for pkg in excludeList:
            if not ':' in pkg:
                excludeList[excludeList.index(pkg)] = pkg + ':default'
        for pkg in excludeList:
            if pkg in pkgList:
                pkgList.remove(pkg)

    if not os.path.exists(opts.buildDir):
        os.makedirs(opts.buildDir)
    if not os.path.exists(opts.prefix):
        os.makedirs(opts.prefix)
    
    os.environ['PATH'] = opts.prefix + '/bin:' + opts.prefix + '/sbin:' + os.environ['PATH']
    os.environ['LD_LIBRARY_PATH'] = opts.prefix + '/lib:' + opts.prefix + '/lib64'
    os.environ['PKG_CONFIG_PATH'] = opts.prefix + '/lib/pkgconfig'
    
    print('Destination directory: ' + opts.prefix)
    
    installPkgs = FindPackages(pkgList)
    print('These packages will be installed:')
    for pkg in pkgList:
        pkgName, pkgTarget = pkg.split(':')
        print('{0}-{1}:{2}'.format(pkgName, installPkgs[pkgName]['version'], pkgTarget))
    
    for pkg in pkgList:
        pkgName, pkgTarget = pkg.split(':')
        BuildPackage(installPkgs[pkgName], pkgTarget)
    
    return 0

if __name__ == '__main__':
    Main()
