//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../../../clang/include/clang/ASTMatchers/ASTMatchFinder.h"
#include "../../../clang/include/clang/Tooling/Core/Diagnostic.h"
#include "../../../clang/include/clang/Tooling/Core/Replacement.h"
#include "../../../llvm/include/llvm/Support/DynamicLibrary.h"
#include "../../../llvm/include/llvm/Support/Registry.h"
#include "../../../llvm/include/llvm/Support/VirtualFileSystem.h"
#include "../ClangTidy.h"
#include "../ClangTidyCheck.h"
#include "../ClangTidyDiagnosticConsumer.h"
#include "../ClangTidyModule.h"
#include "../ClangTidyOptions.h"
#include "../ClangTidyProfiling.h"
#include "../FileExtensionsSet.h"
#include "../NoLintDirectiveHandler.h"
#include "ExceptionEscapeCheck.h"
#include "UseDefaultNoneCheck.h"

namespace clang::tidy {
namespace openmp {
namespace {

/// This module is for OpenMP-specific checks.
class OpenMPModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<ExceptionEscapeCheck>(
        "openmp-exception-escape");
    CheckFactories.registerCheck<UseDefaultNoneCheck>(
        "openmp-use-default-none");
  }
};

} // namespace

// Register the OpenMPTidyModule using this statically initialized variable.
static ClangTidyModuleRegistry::Add<OpenMPModule>
    X("openmp-module", "Adds OpenMP-specific checks.");

} // namespace openmp

// This anchor is used to force the linker to link in the generated object file
// and thus register the OpenMPModule.
volatile int OpenMPModuleAnchorSource = 0; // NOLINT(misc-use-internal-linkage)

} // namespace clang::tidy
