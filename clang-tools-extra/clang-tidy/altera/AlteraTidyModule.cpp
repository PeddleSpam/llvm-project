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
#include "IdDependentBackwardBranchCheck.h"
#include "KernelNameRestrictionCheck.h"
#include "SingleWorkItemBarrierCheck.h"
#include "StructPackAlignCheck.h"
#include "UnrollLoopsCheck.h"

using namespace clang::ast_matchers;

namespace clang::tidy {
namespace altera {
namespace {

class AlteraModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<IdDependentBackwardBranchCheck>(
        "altera-id-dependent-backward-branch");
    CheckFactories.registerCheck<KernelNameRestrictionCheck>(
        "altera-kernel-name-restriction");
    CheckFactories.registerCheck<SingleWorkItemBarrierCheck>(
        "altera-single-work-item-barrier");
    CheckFactories.registerCheck<StructPackAlignCheck>(
        "altera-struct-pack-align");
    CheckFactories.registerCheck<UnrollLoopsCheck>("altera-unroll-loops");
  }
};

} // namespace
} // namespace altera

// Register the AlteraTidyModule using this statically initialized variable.
static ClangTidyModuleRegistry::Add<altera::AlteraModule>
    X("altera-module", "Adds Altera FPGA OpenCL lint checks.");

// This anchor is used to force the linker to link in the generated object file
// and thus register the AlteraModule.
volatile int AlteraModuleAnchorSource = 0; // NOLINT(misc-use-internal-linkage)

} // namespace clang::tidy
