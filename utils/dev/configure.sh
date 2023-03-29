#===- utils/dev/configure.sh ----------------------------------------------===//
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
#===-----------------------------------------------------------------------===//
#
# Command used to configure LLVM in the Docker container
#
# Consider increasing LLVM_PARALLEL_LINK_JOBS depending on your machine.
#
# If you have it, it might be better to build with clang - set CMAKE_C_COMPILER
# and CMAKE_CXX_COMPILER if you want to do that.
#
# Most of the options are self-explanatory. We set BUILD_SHARED_LIBS,
# LLVM_USE_SPLIT_DWARF, and LLVM_OPTIMIZED_TABLEGEN to reduce File IO and save
# on build time.
#
# See:
# https://github.com/lowRISC/riscv-llvm/blob/master/docs/01-intro-and-building-llvm.mkd 
#
#===-----------------------------------------------------------------------===//

cmake -G Ninja -DLLVM_PARALLEL_LINK_JOBS=1 \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_SHARED_LIBS=True -DLLVM_USE_SPLIT_DWARF=True -DLLVM_OPTIMIZED_TABLEGEN=ON \
    -DLLVM_TARGETS_TO_BUILD="" -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD="LC32" \
    -DLLVM_BUILD_TESTS=ON -DLLVM_ENABLE_PROJECTS="clang;lld" \
    ../llvm
