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
#include "../utils/IncludeSorter.h"
#include "../utils/UseRangesCheck.h"
#include "UseRangesCheck.h"
#include "UseToStringCheck.h"
using namespace clang::ast_matchers;

namespace clang::tidy {
namespace boost {
namespace {

class BoostModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<UseRangesCheck>("boost-use-ranges");
    CheckFactories.registerCheck<UseToStringCheck>("boost-use-to-string");
  }
};

} // namespace

// Register the BoostModule using this statically initialized variable.
static ClangTidyModuleRegistry::Add<BoostModule> X("boost-module",
                                                   "Add boost checks.");

} // namespace boost

// This anchor is used to force the linker to link in the generated object file
// and thus register the BoostModule.
volatile int BoostModuleAnchorSource = 0; // NOLINT(misc-use-internal-linkage)

} // namespace clang::tidy
