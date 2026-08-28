// RUN: %clang_cc1 -triple x86_64-pc-win32 \
// RUN:     -fms-extensions -fms-compatibility -fms-compatibility-version=17.00 \
// RUN:     -ffreestanding -fsyntax-only -Werror -Wsystem-headers \
// RUN:     -isystem %S/Inputs/include %s

#include "../../../../../sie/llvm-project/build/Release/lib/clang/24/include/intrin0.h"
#include "../../../../../sie/llvm-project/build/Release/lib/clang/24/include/mm_malloc.h"
#include <immintrin.h>

#pragma clang attribute push(__attribute__((target("avx"))), apply_to=function)
#include <intrin.h>
#pragma clang attribute pop
