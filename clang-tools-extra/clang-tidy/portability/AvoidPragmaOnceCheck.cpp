//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AvoidPragmaOnceCheck.h"
#include "../ClangTidyDiagnosticConsumer.h"
#include "../ClangTidyOptions.h"
#include "../ClangTidyProfiling.h"
#include "../FileExtensionsSet.h"
#include "../NoLintDirectiveHandler.h"

#include "clang/Lex/Preprocessor.h"

namespace clang::tidy::portability {

namespace {

class PragmaOnceCallbacks : public PPCallbacks {
public:
  PragmaOnceCallbacks(AvoidPragmaOnceCheck *Check, const SourceManager &SM)
      : Check(Check), SM(SM) {}
  void PragmaDirective(SourceLocation Loc,
                       PragmaIntroducerKind Introducer) override {
    auto Str = StringRef(SM.getCharacterData(Loc));
    if (!Str.consume_front("#"))
      return;
    Str = Str.trim();
    if (!Str.consume_front("pragma"))
      return;
    Str = Str.trim();
    if (Str.starts_with("once"))
      Check->diag(Loc,
                  "avoid 'pragma once' directive; use include guards instead");
  }

private:
  AvoidPragmaOnceCheck *Check;
  const SourceManager &SM;
};

} // namespace

void AvoidPragmaOnceCheck::registerPPCallbacks(const SourceManager &SM,
                                               Preprocessor *PP,
                                               Preprocessor *ModuleExpanderPP) {
  PP->addPPCallbacks(std::make_unique<PragmaOnceCallbacks>(this, SM));
}

} // namespace clang::tidy::portability
