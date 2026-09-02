//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AnonymousNamespaceInHeaderCheck.h"
#include "../../../clang/include/clang/Tooling/Core/Diagnostic.h"
#include "../../../llvm/include/llvm/Support/VirtualFileSystem.h"
#include "../ClangTidyDiagnosticConsumer.h"
#include "../ClangTidyOptions.h"
#include "../ClangTidyProfiling.h"
#include "../FileExtensionsSet.h"
#include "../NoLintDirectiveHandler.h"
#include "../utils/FileExtensionsUtils.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::misc {

AnonymousNamespaceInHeaderCheck::AnonymousNamespaceInHeaderCheck(
    StringRef Name, ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context) {}

void AnonymousNamespaceInHeaderCheck::registerMatchers(
    ast_matchers::MatchFinder *Finder) {
  Finder->addMatcher(namespaceDecl(isAnonymous()).bind("anonymousNamespace"),
                     this);
}

void AnonymousNamespaceInHeaderCheck::check(
    const MatchFinder::MatchResult &Result) {
  const auto *N = Result.Nodes.getNodeAs<NamespaceDecl>("anonymousNamespace");
  const SourceLocation Loc = N->getBeginLoc();
  if (!Loc.isValid())
    return;

  if (utils::isPresumedLocInHeaderFile(Loc, *Result.SourceManager,
                                       getHeaderFileExtensions()))
    diag(Loc, "do not use unnamed namespaces in header files");
}

} // namespace clang::tidy::misc
