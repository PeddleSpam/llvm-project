//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../../../clang/include/clang/Tooling/Core/Diagnostic.h"
#include "../../../clang/include/clang/Tooling/Core/Replacement.h"
#include "../../../llvm/include/llvm/Support/Errc.h"
#include "../../../llvm/include/llvm/Support/ExtensibleRTTI.h"
#include "../../../llvm/include/llvm/Support/FileSystem.h"
#include "../../../llvm/include/llvm/Support/Path.h"
#include "../../../llvm/include/llvm/Support/SourceMgr.h"
#include "../../../llvm/include/llvm/Support/VirtualFileSystem.h"
#include "../ClangTidy.h"
#include "../ClangTidyDiagnosticConsumer.h"
#include "../ClangTidyModule.h"
#include "../ClangTidyOptions.h"
#include "../ClangTidyProfiling.h"
#include "../FileExtensionsSet.h"
#include "../NoLintDirectiveHandler.h"
#include "BufferDerefCheck.h"
#include "TypeMismatchCheck.h"

namespace clang::tidy {
namespace mpi {
namespace {

class MPIModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<BufferDerefCheck>("mpi-buffer-deref");
    CheckFactories.registerCheck<TypeMismatchCheck>("mpi-type-mismatch");
  }
};

} // namespace
} // namespace mpi

// Register the MPITidyModule using this statically initialized variable.
static ClangTidyModuleRegistry::Add<mpi::MPIModule>
    X("mpi-module", "Adds MPI clang-tidy checks.");

// This anchor is used to force the linker to link in the generated object file
// and thus register the MPIModule.
volatile int MPIModuleAnchorSource = 0; // NOLINT(misc-use-internal-linkage)

} // namespace clang::tidy
