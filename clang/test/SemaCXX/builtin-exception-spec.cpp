// RUN: %clang_cc1 -isystem %S/Inputs -fsyntax-only -verify %s
// RUN: %clang_cc1 -isystem %S/Inputs -fsyntax-only -verify -std=c++1z %s
// expected-no-diagnostics
#include "../../../../../sie/llvm-project/build/Release/lib/clang/24/include/vadefs.h"
#include <malloc.h>

extern "C" {
void *malloc(__SIZE_TYPE__);
}
