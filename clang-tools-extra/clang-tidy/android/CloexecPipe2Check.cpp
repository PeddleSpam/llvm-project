//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CloexecPipe2Check.h"
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

void CloexecPipe2Check::registerMatchers(MatchFinder *Finder) {
  registerMatchersImpl(
      Finder, functionDecl(returns(isInteger()), hasName("pipe2"),
                           hasParameter(0, hasType(pointsTo(isInteger()))),
                           hasParameter(1, hasType(isInteger()))));
}

void CloexecPipe2Check::check(const MatchFinder::MatchResult &Result) {
  insertMacroFlag(Result, /*MacroFlag=*/"O_CLOEXEC", /*ArgPos=*/1);
}

} // namespace clang::tidy::android
