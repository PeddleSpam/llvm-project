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
#include "../ClangTidyCheck.h"
#include "../ClangTidyDiagnosticConsumer.h"
#include "../ClangTidyModule.h"
#include "../ClangTidyOptions.h"
#include "../ClangTidyProfiling.h"
#include "../FileExtensionsSet.h"
#include "../NoLintDirectiveHandler.h"
#include "AvoidSpinlockCheck.h"
#include "DispatchOnceNonstaticCheck.h"

namespace clang::tidy {
namespace darwin {
namespace {

class DarwinModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<AvoidSpinlockCheck>("darwin-avoid-spinlock");
    CheckFactories.registerCheck<DispatchOnceNonstaticCheck>(
        "darwin-dispatch-once-nonstatic");
  }
};

} // namespace
} // namespace darwin

// Register the DarwinTidyModule using this statically initialized variable.
static ClangTidyModuleRegistry::Add<darwin::DarwinModule>
    X("darwin-module", "Adds Darwin-specific lint checks.");

// This anchor is used to force the linker to link in the generated object file
// and thus register the DarwinModule.
volatile int DarwinModuleAnchorSource = 0; // NOLINT(misc-use-internal-linkage)

} // namespace clang::tidy
