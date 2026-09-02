//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CloexecSocketCheck.h"
#include "../../../clang/include/clang/Tooling/Core/Diagnostic.h"
#include "../../../llvm/include/llvm/Support/VirtualFileSystem.h"
#include "../ClangTidyDiagnosticConsumer.h"
#include "../ClangTidyOptions.h"
#include "../ClangTidyProfiling.h"
#include "../FileExtensionsSet.h"
#include "../NoLintDirectiveHandler.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::android {

void CloexecSocketCheck::registerMatchers(MatchFinder *Finder) {
  registerMatchersImpl(
      Finder, functionDecl(isExternC(), returns(isInteger()), hasName("socket"),
                           hasParameter(0, hasType(isInteger())),
                           hasParameter(1, hasType(isInteger())),
                           hasParameter(2, hasType(isInteger()))));
}

void CloexecSocketCheck::check(const MatchFinder::MatchResult &Result) {
  insertMacroFlag(Result, /*MacroFlag=*/"SOCK_CLOEXEC", /*ArgPos=*/1);
}

} // namespace clang::tidy::android
