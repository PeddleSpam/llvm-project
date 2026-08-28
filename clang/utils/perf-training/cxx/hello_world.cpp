// RUN: %clang_cpp -c %s
// RUN: %clang_cpp_skip_driver -Wall -pedantic -c %s
#include "../../../../../../sie/llvm-project/build/Release/lib/clang/24/include/float.h"
#include "../../../../../../sie/llvm-project/build/Release/lib/clang/24/include/intrin0.h"
#include "../../../../../../sie/llvm-project/build/Release/lib/clang/24/include/mm_malloc.h"
#include "../../../../../../sie/llvm-project/build/Release/lib/clang/24/include/stdint.h"
#include "../../../../../../sie/llvm-project/build/Release/lib/clang/24/include/yvals_core.h"
#include <iostream>

int main(int, char**) {
  std::cout << "Hello, World!";
  return 0;
}
