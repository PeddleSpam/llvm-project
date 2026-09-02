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
#include "../../../llvm/include/llvm/Support/Registry.h"
#include "../../../llvm/include/llvm/Support/VirtualFileSystem.h"
#include "../ClangTidy.h"
#include "../ClangTidyDiagnosticConsumer.h"
#include "../ClangTidyModule.h"
#include "../ClangTidyOptions.h"
#include "../ClangTidyProfiling.h"
#include "../FileExtensionsSet.h"
#include "../NoLintDirectiveHandler.h"
#include "MustCheckErrsCheck.h"

namespace clang::tidy {
namespace linuxkernel {
namespace {

/// This module is for checks specific to the Linux kernel.
class LinuxKernelModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<MustCheckErrsCheck>(
        "linuxkernel-must-check-errs");
  }
};

} // namespace

// Register the LinuxKernelTidyModule using this statically initialized
// variable.
static ClangTidyModuleRegistry::Add<LinuxKernelModule>
    X("linux-module", "Adds checks specific to the Linux kernel.");
} // namespace linuxkernel

// This anchor is used to force the linker to link in the generated object file
// and thus register the LinuxKernelModule.
// NOLINTNEXTLINE(misc-use-internal-linkage)
volatile int LinuxKernelModuleAnchorSource = 0;

} // namespace clang::tidy
