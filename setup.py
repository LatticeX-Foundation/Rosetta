import glob
import shutil
import os
from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext
import sys
import setuptools

# must install tensorflow first
import tensorflow as tf
TF_INCS = tf.sysconfig.get_include()
TF_LIBS = tf.sysconfig.get_lib()
TF_CFLG = tf.sysconfig.get_compile_flags()
TF_LFLG = tf.sysconfig.get_link_flags()
#
print('TF_INCS', TF_INCS)
print('TF_LIBS', TF_LIBS)
print('TF_CFLG', TF_CFLG)
print('TF_LFLG', TF_LFLG)
print('-' * 80)


class get_pybind_include(object):
    """Helper class to determine the pybind11 include path

    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the ``get_include()``
    method can be invoked. """

    def __init__(self, user=False):
        self.user = user

    def __str__(self):
        import pybind11
        return pybind11.get_include(self.user)


# As of Python 3.6, CCompiler has a `has_flag` method.
# cf http://bugs.python.org/issue26689
def has_flag(compiler, flagname):
    """Return a boolean indicating whether a flag name is supported on
    the specified compiler.
    """
    import tempfile
    with tempfile.NamedTemporaryFile('w', suffix='.cpp') as f:
        f.write('int main (int argc, char **argv) { return 0; }')
        try:
            compiler.compile([f.name], extra_postargs=[flagname])
        except setuptools.distutils.errors.CompileError:
            return False
    return True


def cpp_flag(compiler):
    """Return the -std=c++[11/14/17] compiler flag.

    The newer version is prefered over c++11 (when it is available).
    """
    flags = ['-std=c++17', '-std=c++14', '-std=c++11']

    for flag in flags:
        if has_flag(compiler, flag):
            return flag

    raise RuntimeError('Unsupported compiler -- at least C++11 support '
                       'is needed!')


class BuildExt(build_ext):
    """A custom build extension for adding compiler-specific options."""
    c_opts = {
        'msvc': ['/EHsc'],
        'unix': [],
    }
    l_opts = {
        'msvc': [],
        'unix': [],
    }

    if sys.platform == 'darwin':
        darwin_opts = ['-stdlib=libc++', '-mmacosx-version-min=10.7']
        c_opts['unix'] += darwin_opts
        l_opts['unix'] += darwin_opts

    def build_extensions(self):
        ct = self.compiler.compiler_type
        opts = self.c_opts.get(ct, [])
        link_opts = self.l_opts.get(ct, [])
        if ct == 'unix':
            opts.append('-DVERSION_INFO="%s"' %
                        self.distribution.get_version())
            opts.append(cpp_flag(self.compiler))
            if has_flag(self.compiler, '-fvisibility=hidden'):
                opts.append('-fvisibility=hidden')

        elif ct == 'msvc':
            opts.append('/DVERSION_INFO=\\"%s\\"' %
                        self.distribution.get_version())
        for ext in self.extensions:
            ext.extra_compile_args = opts
            ext.extra_link_args = link_opts
        build_ext.build_extensions(self)


"""
From here.
"""

__version__ = '0.1.0'

root_dir = os.path.dirname(os.path.abspath(__file__))
include_dirs = []
include_dirs.append('.')
include_dirs.append('include')


# tf includes
include_dirs.append(TF_INCS)

# mpc includes
ccdir = root_dir+"/cc"
include_dirs.append(ccdir+"/modules/common/include")
include_dirs.append(ccdir+"/modules/common/include/utils")
include_dirs.append(ccdir+"/modules/io/include")
include_dirs.append(ccdir+"/modules/protocol/mpc/include")
include_dirs.append(ccdir+"/modules/protocol/mpc/src/snn")


# thirdparty includes
include_dirs.append(ccdir+"/third_party/rapidjson/include")
include_dirs.append(ccdir+"/third_party/pybind11/include")

# libraries search path
library_dirs = ['.']
library_dirs.append(TF_LIBS)
library_dirs.append('./lib')
library_dirs.append("./build/lib")

# compile flags and definitions
extra_cflags = []
#extra_cflags += TF_CFLG
extra_cflags.append(TF_CFLG[0])
extra_cflags.append('-D_GLIBCXX_USE_CXX11_ABI=1')
extra_cflags.append('-DSML_USE_UINT64=1')  # mpc
extra_cflags.append('-fPIC')  # general
extra_cflags.append('-Wno-unused-function')  # general


extra_lflags = []
extra_lflags += TF_LFLG

link_rpath = "$ORIGIN/..:$ORIGIN"
# for i in range(len(sys.path)):
#     if i == len(sys.path)-1 or sys.path[i] == '':
#         link_rpath += sys.path[i]
#     else:
#         link_rpath += sys.path[i] + ':'
extra_lflags.append('-Wl,-rpath={}'.format(link_rpath))

print('extra_lflags', extra_lflags)
print('extra_cflags', extra_cflags)
print('library_dirs', library_dirs)
print('include_dirs', include_dirs)

ext_modules = [
    Extension(
        'latticex/_rosetta',
        ['cc/tf/misc/_rosetta.cc'],
        include_dirs=include_dirs,
        libraries=['tf-mpcop', 'tf-dpass', 'mpc-op', 'mpc-io'],
        library_dirs=library_dirs,
        extra_compile_args=extra_cflags,
        extra_link_args=extra_lflags,
        language='c++'
    ),
    # others here
]

# copy libs to latticex
so_libs = glob.glob('build/lib/lib*.so')
for file_name in so_libs:
    shutil.copy(file_name, "python/latticex/")

setup(
    name='latticex',
    version=__version__,
    author='LatticeX Foundation',
    author_email='support@platon.network',
    url='',
    description='LatticeX Rosetta Based On Tensorflow',
    long_description='',
    package_dir={'': 'python'},  # where to find package
    packages=find_packages(
        'python', exclude=['contrib', 'build', 'docs', 'tests', 'test-cases']),
    ext_modules=ext_modules,
    # Add in any packaged data.
    include_package_data=True,
    install_requires=['pybind11>=2.4', 'pandas'],
    setup_requires=['pybind11>=2.4'],
    #cmdclass={'build_ext': BuildExt},
    zip_safe=False,
)

# after setup clean the libs
so_libs = glob.glob('python/latticex/lib*.so')
for file_name in so_libs:
    os.remove(file_name)
