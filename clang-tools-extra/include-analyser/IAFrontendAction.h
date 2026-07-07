#ifndef INCLUDE_ANALYSER_IAFRONTENDACTION_H
#define INCLUDE_ANALYSER_IAFRONTENDACTION_H

#include "clang/Frontend/FrontendAction.h"

#include "InclusionCollector.h"
#include "IAConsumer.h"
#include "IAOptions.h"

namespace clang {
namespace include_analyser {

class IAFrontendAction : public clang::ASTFrontendAction {
public:
  IAFrontendAction(llvm::StringMap<std::string>& editedFiles,
                   IAOptions& options);

  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
      clang::CompilerInstance& compiler, llvm::StringRef file) override;

  bool BeginInvocation(clang::CompilerInstance &compiler) override;
  bool BeginSourceFileAction(clang::CompilerInstance &compiler) override;
  void ExecuteAction() override;
  void EndSourceFileAction() override;

private:
  using PathType = InclusionCollector::PathType;

  IAOptions* m_options;
  IAConsumer* m_consumer;
  InclusionCollector* m_collector;
  llvm::StringMap<std::string>* m_editedFiles;
};

} // namespace include_analyser
} // namespace clang

#endif // INCLUDE_ANALYSER_IAFRONTENDACTION_H
