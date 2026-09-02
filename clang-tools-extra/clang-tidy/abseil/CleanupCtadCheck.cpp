//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CleanupCtadCheck.h"
#include "../../../clang/include/clang/Tooling/Core/Diagnostic.h"
#include "../../../clang/include/clang/Tooling/Transformer/Transformer.h"
#include "../../../llvm/include/llvm/Support/VirtualFileSystem.h"
#include "../ClangTidyCheck.h"
#include "../ClangTidyDiagnosticConsumer.h"
#include "../ClangTidyOptions.h"
#include "../ClangTidyProfiling.h"
#include "../FileExtensionsSet.h"
#include "../NoLintDirectiveHandler.h"
#include "../utils/IncludeInserter.h"
#include "../utils/IncludeSorter.h"
#include "clang/Tooling/Transformer/Stencil.h"

using namespace ::clang::ast_matchers;
using namespace ::clang::transformer;

namespace clang::tidy::abseil {

static RewriteRuleWith<std::string> cleanupCtadCheckImpl() {
  const auto WarningMessage =
      cat("prefer absl::Cleanup's class template argument "
          "deduction pattern in C++17 and higher");

  return makeRule(
      declStmt(hasSingleDecl(varDecl(
          hasType(autoType()), hasTypeLoc(typeLoc().bind("auto_type_loc")),
          hasInitializer(hasDescendant(
              callExpr(callee(functionDecl(hasName("absl::MakeCleanup"))),
                       argumentCountIs(1))
                  .bind("make_cleanup_call")))))),
      {changeTo(node("auto_type_loc"), cat("absl::Cleanup")),
       changeTo(node("make_cleanup_call"), cat(callArgs("make_cleanup_call")))},
      WarningMessage);
}

CleanupCtadCheck::CleanupCtadCheck(StringRef Name, ClangTidyContext *Context)
    : utils::TransformerClangTidyCheck(cleanupCtadCheckImpl(), Name, Context) {}

} // namespace clang::tidy::abseil
