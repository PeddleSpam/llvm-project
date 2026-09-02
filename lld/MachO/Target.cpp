//===- Target.cpp ---------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Target.h"
#include "../../libunwind/include/mach-o/compact_unwind_encoding.h"

using namespace lld;
using namespace lld::macho;

TargetInfo *macho::target = nullptr;
