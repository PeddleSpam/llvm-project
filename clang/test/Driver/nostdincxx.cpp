// RUN: not %clangxx -nostdinc %s 2>&1 | FileCheck %s
// RUN: not %clangxx -nostdinc++ %s 2>&1 | FileCheck %s
// RUN: not %clangxx -nostdlibinc %s 2>&1 | FileCheck %s
// RUN: not %clangxx --target=x86_64-unknown-unknown-gnu -fsyntax-only -nostdinc -nostdinc++ %s 2>&1 | FileCheck /dev/null --implicit-check-not=-Wunused-command-line-argument
// RUN: not %clangxx --target=riscv64-unknown-elf -fsyntax-only -nostdinc -nostdinc++ %s 2>&1 | FileCheck /dev/null --implicit-check-not=-Wunused-command-line-argument
// CHECK: 'vector' file not found
#include "../../../../../sie/llvm-project/build/Release/lib/clang/24/include/float.h"
#include "../../../../../sie/llvm-project/build/Release/lib/clang/24/include/intrin0.h"
#include "../../../../../sie/llvm-project/build/Release/lib/clang/24/include/stdint.h"
#include <vector>

// MSVC, PS4, PS5 have C++ headers in the same directory as C headers.
// UNSUPPORTED: ms-sdk, target={{.*-(ps4|ps5)}}
