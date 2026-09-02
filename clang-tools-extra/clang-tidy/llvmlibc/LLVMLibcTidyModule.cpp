//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../../../clang/include/clang/ASTMatchers/ASTMatchFinder.h"
#include "../../../clang/include/clang/Basic/Module.h"
#include "../../../clang/include/clang/Lex/ModuleLoader.h"
#include "../../../clang/include/clang/Lex/PPCallbacks.h"
#include "../../../clang/include/clang/Tooling/Core/Diagnostic.h"
#include "../../../clang/include/clang/Tooling/Core/Replacement.h"
#include "../../../llvm/include/llvm/Support/Chrono.h"
#include "../../../llvm/include/llvm/Support/DynamicLibrary.h"
#include "../../../llvm/include/llvm/Support/Errc.h"
#include "../../../llvm/include/llvm/Support/ExtensibleRTTI.h"
#include "../../../llvm/include/llvm/Support/FileSystem.h"
#include "../../../llvm/include/llvm/Support/MD5.h"
#include "../../../llvm/include/llvm/Support/Path.h"
#include "../../../llvm/include/llvm/Support/Registry.h"
#include "../../../llvm/include/llvm/Support/SMLoc.h"
#include "../../../llvm/include/llvm/Support/SourceMgr.h"
#include "../../../llvm/include/llvm/Support/VirtualFileSystem.h"
#include "../ClangTidy.h"
#include "../ClangTidyDiagnosticConsumer.h"
#include "../ClangTidyModule.h"
#include "../ClangTidyOptions.h"
#include "../ClangTidyProfiling.h"
#include "../FileExtensionsSet.h"
#include "../GlobList.h"
#include "../NoLintDirectiveHandler.h"
#include "CalleeNamespaceCheck.h"
#include "ImplementationInNamespaceCheck.h"
#include "InlineFunctionDeclCheck.h"
#include "RestrictSystemLibcHeadersCheck.h"

namespace clang::tidy {
namespace llvm_libc {
namespace {

class LLVMLibcModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<CalleeNamespaceCheck>(
        "llvmlibc-callee-namespace");
    CheckFactories.registerCheck<ImplementationInNamespaceCheck>(
        "llvmlibc-implementation-in-namespace");
    CheckFactories.registerCheck<InlineFunctionDeclCheck>(
        "llvmlibc-inline-function-decl");
    CheckFactories.registerCheck<RestrictSystemLibcHeadersCheck>(
        "llvmlibc-restrict-system-libc-headers");
  }
};

} // namespace

// Register the LLVMLibcTidyModule using this statically initialized variable.
static ClangTidyModuleRegistry::Add<LLVMLibcModule>
    X("llvmlibc-module", "Adds LLVM libc standards checks.");

} // namespace llvm_libc

// This anchor is used to force the linker to link in the generated object file
// and thus register the LLVMLibcModule.
// NOLINTNEXTLINE(misc-use-internal-linkage)
volatile int LLVMLibcModuleAnchorSource = 0;

} // namespace clang::tidy
