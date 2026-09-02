// RUN: %clang_cc1 %s -I%S -fvisibility=hidden -triple x86_64-linux-gnu -emit-llvm -o - | FileCheck %s

#include "../../../../../sie/llvm-project/build/Release/lib/clang/24/include/limits.h"
#include "../../../../../sie/llvm-project/build/Release/lib/clang/24/include/stddef.h"
#include "../../../../../sie/llvm-project/build/Release/lib/clang/24/include/stdint.h"
#include <stdarg.h>
#include <typeinfo>

// If struct __va_list_tag did not explicitly have default visibility, then
// under -fvisibility=hidden the type of function f, due to its va_list (aka
// __builtin_va_list, aka __va_list_tag (*)[1]) parameter would be hidden:

// CHECK: @_ZTIFvP13__va_list_tagE = linkonce_odr constant
// CHECK: @_ZTSFvP13__va_list_tagE = linkonce_odr constant
void f(va_list) { (void)typeid(f); }
