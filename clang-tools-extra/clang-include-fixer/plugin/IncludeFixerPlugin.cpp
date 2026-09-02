//===- IncludeFixerPlugin.cpp - clang-include-fixer as a clang plugin -----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../../../clang/include/clang/Format/Format.h"
#include "../../../clang/include/clang/Frontend/ASTUnit.h"
#include "../../../clang/include/clang/Frontend/PrecompiledPreamble.h"
#include "../../../clang/include/clang/Frontend/StandaloneDiagnostic.h"
#include "../../../clang/include/clang/Lex/PreprocessingRecord.h"
#include "../../../clang/include/clang/Sema/CodeCompleteConsumer.h"
#include "../../../clang/include/clang/Serialization/ASTWriter.h"
#include "../../../clang/include/clang/Tooling/ArgumentsAdjusters.h"
#include "../../../clang/include/clang/Tooling/Tooling.h"
#include "../IncludeFixer.h"
#include "../IncludeFixerContext.h"
#include "../SymbolIndexManager.h"
#include "../YamlSymbolIndex.h"
#include "../find-all-symbols/SymbolInfo.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Parse/ParseAST.h"

namespace clang {
namespace include_fixer {

/// The core include fixer plugin action. This just provides the AST consumer
/// and command line flag parsing for using include fixer as a clang plugin.
class ClangIncludeFixerPluginAction : public PluginASTAction {
  /// ASTConsumer to keep the symbol index alive. We don't really need an
  /// ASTConsumer for this plugin (everything is funneled on the side through
  /// Sema) but we have to keep the symbol index alive until sema is done.
  struct ASTConsumerManagerWrapper : public ASTConsumer {
    ASTConsumerManagerWrapper(std::shared_ptr<SymbolIndexManager> SIM)
        : SymbolIndexMgr(std::move(SIM)) {}
    std::shared_ptr<SymbolIndexManager> SymbolIndexMgr;
  };

public:
  explicit ClangIncludeFixerPluginAction()
      : SymbolIndexMgr(std::make_shared<SymbolIndexManager>()),
        SemaSource(new IncludeFixerSemaSource(*SymbolIndexMgr,
                                              /*MinimizeIncludePaths=*/true,
                                              /*GenerateDiagnostics=*/true)) {}

  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI, StringRef InFile) override {
    CI.setExternalSemaSource(SemaSource);
    SemaSource->setFilePath(InFile);
    SemaSource->setCompilerInstance(&CI);
    return std::make_unique<ASTConsumerManagerWrapper>(SymbolIndexMgr);
  }

  void ExecuteAction() override {} // Do nothing.

  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &Args) override {
    StringRef DB = "yaml";
    StringRef Input;

    // Parse the extra command line args.
    // FIXME: This is very limited at the moment.
    for (StringRef Arg : Args) {
      if (Arg.starts_with("-db="))
        DB = Arg.substr(strlen("-db="));
      else if (Arg.starts_with("-input="))
        Input = Arg.substr(strlen("-input="));
    }

    std::string InputFile =
        std::string(CI.getFrontendOpts().Inputs[0].getFile());
    auto CreateYamlIdx = [=]() -> std::unique_ptr<include_fixer::SymbolIndex> {
      llvm::ErrorOr<std::unique_ptr<include_fixer::YamlSymbolIndex>> SymbolIdx(
          nullptr);
      if (DB == "yaml") {
        if (!Input.empty()) {
          SymbolIdx = include_fixer::YamlSymbolIndex::createFromFile(Input);
        } else {
          // If we don't have any input file, look in the directory of the first
          // file and its parents.
          SmallString<128> AbsolutePath(tooling::getAbsolutePath(InputFile));
          StringRef Directory = llvm::sys::path::parent_path(AbsolutePath);
          SymbolIdx = include_fixer::YamlSymbolIndex::createFromDirectory(
              Directory, "find_all_symbols_db.yaml");
        }
      }
      return std::move(*SymbolIdx);
    };

    SymbolIndexMgr->addSymbolIndex(std::move(CreateYamlIdx));
    return true;
  }

private:
  std::shared_ptr<SymbolIndexManager> SymbolIndexMgr;
  IntrusiveRefCntPtr<IncludeFixerSemaSource> SemaSource;
};
} // namespace include_fixer
} // namespace clang

// This anchor is used to force the linker to link in the generated object file
// and thus register the include fixer plugin.
volatile int ClangIncludeFixerPluginAnchorSource = 0;

static clang::FrontendPluginRegistry::Add<
    clang::include_fixer::ClangIncludeFixerPluginAction>
    X("clang-include-fixer", "clang-include-fixer");
