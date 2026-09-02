//===- LLVMPasses.cpp - C API for LLVM Dialect Passes ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../../../include/mlir/Dialect/LLVMIR/LLVMAttrs.h"
#include "../../../include/mlir/Dialect/LLVMIR/Transforms/AddComdats.h"
#include "../../../include/mlir/Dialect/LLVMIR/Transforms/LegalizeForExport.h"
#include "../../../include/mlir/Dialect/LLVMIR/Transforms/RequestCWrappers.h"
#include "mlir/CAPI/Pass.h"

// Must include the declarations as they carry important visibility attributes.
#include "mlir/Dialect/LLVMIR/Transforms/Passes.capi.h.inc"
using namespace mlir;
using namespace mlir::LLVM;

#ifdef __cplusplus
extern "C" {
#endif

#include "mlir/Dialect/LLVMIR/Transforms/Passes.capi.cpp.inc"

#ifdef __cplusplus
}
#endif
