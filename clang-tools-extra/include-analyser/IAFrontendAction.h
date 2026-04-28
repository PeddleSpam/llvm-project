#ifndef INCLUDE_ANALYSER_IAFRONTENDACTION_H
#define INCLUDE_ANALYSER_IAFRONTENDACTION_H

#include "clang/Frontend/FrontendAction.h"

#include "InclusionCollector.h"
#include "IAConsumer.h"

class IAFrontendAction : public clang::ASTFrontendAction {
public:
  IAFrontendAction(llvm::raw_ostream& output);

  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance& compiler, llvm::StringRef file) override;

  bool BeginSourceFileAction(clang::CompilerInstance &compiler) override;
  void EndSourceFileAction() override;

private:
  using PathType = InclusionCollector::PathType;

  IAConsumer* m_consumer;
  llvm::raw_ostream* m_output;
  InclusionCollector* m_collector;
  clang::CompilerInstance* m_compiler;

  std::set<PathType> m_optimalIncludes;
};

#endif // INCLUDE_ANALYSER_IAFRONTENDACTION_H
