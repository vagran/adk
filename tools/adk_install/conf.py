# This file describes packages building and installation configuration

# Order corresponds to installation order. Each entry is string in format
# <package name>[:<target name>]
packages = [
    # Development tools
    'binutils:avr',
    'gcc:avr',
    'avr-libc',
    'flex',
    'bison',
    'gmp',
    'mpfr',
    'mpc',
    'binutils:native',
    'gcc:native',
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
    'atkmm',
    'pangomm',
    'cairomm',
    'gtkmm'
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
                '--without-included-gettext --disable-checking ' +
                '--disable-libada --disable-libssp --disable-libquadmath --disable-libgomp'
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
    }
}
