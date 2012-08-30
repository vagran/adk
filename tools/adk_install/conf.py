# This file describes packages building and installation configuration. Some
# packages should be available in the environment. Known dependencies:
# m4
# gawk
# gcc-multilib

# May be required on 64-bits host:
# sudo ln -s /usr/lib/x86_64-linux-gnu /usr/lib64

# Order corresponds to installation order. Each entry is string in format
# <package name>[:<target name>]
packages = [
    # Development tools
    'Python',
    'gmp',
    'mpfr',
    'mpc',
    'flex',
    'bison',
    'binutils:avr',
    'gcc:avr',
    'avr-libc',
    'binutils:native',
    'gcc:native',
    # Tools for cross-compilation targeted to Windows
#    'binutils:windows'
#    'w32api',
#    'mingwrt',
#    'gcc:windows',
    'doxygen',
    # Run-time components
    'freetype',
    'libffi',
    'glib',
    'pixman',
    'cairo',
    'atk',
    'pango',
    'gdk-pixbuf',
    'gtk+',
    'libsigc++',
    'glibmm',
    'cairomm',
    'atkmm',
    'pangomm',
    'gtkmm',
    'valgrind'
]

# Build/install options for each package. Each entry key is package name, payload
# is dictionary with target names. Name 'default' used for default target. Target
# are various configuration options. Installation variables can be referenced as
# '${<variable name>}' in options.
# Valid options:
# 'config-params' - additional arguments to 'configure' script.
# 'no-obj-dir' - do not use separate directory for object files when True
# 'post-install-cmd' - execute command after installation
opts = {
    'Python': {
        'default': {
            'config-params': '--enable-shared'
        }
    },
    'avr-libc': {
        'default': {
            'config-params': '--host=avr'
        }
    },
    'mpfr': {
        'default': {
            'config-params': '--with-gmp=${PREFIX}'
        }
    },
    'mpc': {
        'default': {
            'config-params': '--with-gmp=${PREFIX} --with-mpfr=${PREFIX}'
        }
    },
    'binutils': {
        'native': {
            'config-params': '--program-prefix=adk-'
        },
        'avr': {
            'config-params': '--target=avr --program-prefix=avr-'
        }
    },
    'gcc': {
        'native': {
            'config-params': '--program-prefix=adk- --disable-bootstrap --disable-libada ' +
                '--disable-libssp --disable-libquadmath --disable-libgomp ' +
                '--enable-languages=c,c++ --with-gmp=${PREFIX} --with-mpfr=${PREFIX} --with-mpc=${PREFIX}'
        },
        'avr': {
            'config-params': '--program-prefix=avr- --target=avr --enable-languages=c,c++ ' +
                '--enable-shared --with-system-zlib --enable-long-long --enable-nls ' +
                '--without-included-gettext --disable-checking --disable-bootstrap ' +
                '--disable-libada --disable-libssp --disable-libquadmath --disable-libgomp ' +
                '--with-gmp=${PREFIX} --with-mpfr=${PREFIX} --with-mpc=${PREFIX}'
        }
    },
    'doxygen': {
        'default': {
            'no-obj-dir': True
        }
    },
    'gdk-pixbuf': {
        'default': {
            'config-params': '--without-libtiff'
        }
    },
    'valgrind': {
        'default': {
            'no-obj-dir': True
        }
    },
}
