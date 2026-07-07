#ifndef INCLUDE_ANALYSER_IAACTIONFACTORY_H
#define INCLUDE_ANALYSER_IAACTIONFACTORY_H

#include <memory>
#include "clang/Tooling/Tooling.h"
#include "IAFrontendAction.h"
#include "IAOptions.h"

namespace clang {
namespace include_analyser {

class IAActionFactory : public clang::tooling::FrontendActionFactory {
public:
  IAActionFactory(IAOptions options);

  std::unique_ptr<clang::FrontendAction> create() override;

  llvm::StringMap<std::string> const &getEditedFiles() const;

private:
  llvm::StringMap<std::string> m_editedFiles;
  IAOptions m_options;
};

} // namespace include_analyser
} // namespace clang

#endif // INCLUDE_ANALYSER_IAACTIONFACTORY_H