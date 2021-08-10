# ==============================================================================
# Copyright 2020 The LatticeX Foundation
# This file is part of the Rosetta library.
#
# The Rosetta library is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The Rosetta library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with the Rosetta library. If not, see <http://www.gnu.org/licenses/>.
# =============================================================================="

"""Rosetta is an open source privacy-preserving machine learning framework.

Rosetta is a privacy-preserving framework based on [TensorFlow](https://www.tensorflow.org). 
It integrates with mainstream privacy-preserving computation technologies, including crypography, 
federated learning and trusted execution environment. Rosetta aims to provide privacy-preserving 
solutions for artificial intellegence without requiring expertise in cryptogprahy, federated 
learning and trusted execution environment. 

Rosetta reuses the APIs of TensorFlow and allows to transfer traditional TensorFlow codes 
into a privacy-preserving manner with minimal changes. E.g., just add the one line to enable Rosetta execution.
"""

import glob
import shutil
import os
from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext
import sys
import setuptools
# REFINE UserWarning: Distutils was imported before Setuptools.
# This usage is discouraged and may exhibit undesirable behaviors or errors.
from distutils import sysconfig

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

build_ext_target = 'latticex/_rosetta'
build_128_mpc = False
if 'ROSETTA_MPC_128' in os.environ and os.environ['ROSETTA_MPC_128'] == 'ON':
    build_ext_target = 'latticex/lib128/_rosetta'
    build_128_mpc = True


DOCLINES = __doc__.split('\n')
__version__ = '1.0.0'

root_dir = os.path.dirname(os.path.abspath(__file__))
include_dirs = []
include_dirs.append(root_dir)
include_dirs.append('.')
include_dirs.append('include')


# tf includes
include_dirs.append(TF_INCS)

# mpc includes
ccdir = root_dir+"/cc"
include_dirs.append(ccdir+"/modules/common/include")
include_dirs.append(ccdir+"/modules/common/include/utils")
include_dirs.append(ccdir+"/modules/protocol/mpc/include")
include_dirs.append(ccdir+"/modules/protocol/mpc/snn/src")
# added in V0.2.0
include_dirs.append(ccdir+"/modules/protocol/public")

# thirdparty includes
include_dirs.append(ccdir+"/third_party/rapidjson/include")
include_dirs.append(ccdir+"/third_party/pybind11/include")
include_dirs.append(ccdir+"/third_party/spdlog/include")
include_dirs.append(ccdir+"/third_party/io/include")

# libraries search path
library_dirs = ['.']
library_dirs.append(TF_LIBS)
if build_128_mpc:
    library_dirs.append("./build128/lib")
else:
    library_dirs.append("./build/lib")

# compile flags and definitions
extra_cflags = []
extra_cflags += TF_CFLG
extra_cflags.append('-DSML_USE_UINT64=1')  # mpc
if build_128_mpc:
    extra_cflags.append('-DROSETTA_MPC_128=1')  # mpc

extra_cflags.append('-fPIC')  # general
extra_cflags.append('-Wno-unused-function')  # general
extra_cflags.append('-Wno-sign-compare')

extra_cflags.append('-std=c++11')  # temp c++11


extra_lflags = []
extra_lflags += TF_LFLG

link_rpath = "$ORIGIN"
extra_lflags.append('-Wl,-rpath={}'.format(link_rpath))

print('extra_lflags', extra_lflags)
print('extra_cflags', extra_cflags)
print('library_dirs', library_dirs)
print('include_dirs', include_dirs)


ext_modules = [
    Extension(
        build_ext_target,
        ['cc/python_export/_rosetta.cc'],
        # cc_files,
        include_dirs=include_dirs,
        libraries=['tf-dpass', 'io', 'protocol-utility',
                   'common', 'protocol-base', 'protocol-api'],
        library_dirs=library_dirs,
        extra_compile_args=extra_cflags,
        extra_link_args=extra_lflags,
        language='c++'
    ),
    # others here
]

# copy 64-bits libs to latticex
so_libs = glob.glob('build/lib/lib*.so')
for file_name in so_libs:
    shutil.copy(file_name, "python/latticex/")

# copy 128-bits libs if exists
if os.path.isdir("build128/lib"):
    # check target lib128 directory
    if not os.path.isdir("python/latticex/lib128"):
        os.mkdir("python/latticex/lib128")
    # copy 128 bits libs
    so_lib128s = glob.glob('build128/lib/lib*.so')
    for file_name in so_lib128s:
        shutil.copy(file_name, "python/latticex/lib128/")

# disable debug
if sys.platform == 'linux' or sys.platform == "darwin":  # remove -g flags
    for k in sysconfig._config_vars.keys():
        if isinstance(sysconfig._config_vars[k], str):
            sysconfig._config_vars[k] = sysconfig._config_vars[k].replace(
                '-g ', ' ')

setup(
    name='latticex-rosetta',
    version=__version__,
    author='LatticeX Foundation',
    author_email='rosetta@latticex.foundation',
    download_url='https://github.com/LatticeX-Foundation/Rosetta',
    description=DOCLINES[0],
    long_description='\n'.join(DOCLINES[2:]),
    package_dir={'': 'python'},  # where to find package
    packages=find_packages(
        'python', exclude=['contrib', 'build', 'docs', 'tests', 'test-cases']),
    ext_modules=ext_modules,
    # Add in any packaged data.
    include_package_data=True,
    install_requires=['numpy', 'pandas', 'sklearn'],
    setup_requires=['pybind11>=2.4'],
    zip_safe=False,
    # PyPI package information.
    classifiers=[
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'Intended Audience :: Education',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved :: GNU Lesser General Public License v3 (LGPLv3)',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Topic :: Scientific/Engineering',
        'Topic :: Scientific/Engineering :: Mathematics',
        'Topic :: Scientific/Engineering :: Artificial Intelligence',
        'Topic :: Software Development',
        'Topic :: Software Development :: Libraries',
        'Topic :: Software Development :: Libraries :: Python Modules',
    ],
    license='LGPLv3',
    keywords='privacy-preserving machine learning',
)
