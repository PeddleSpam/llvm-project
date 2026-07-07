#include "IAActionFactory.h"

namespace clang {
namespace include_analyser {

IAActionFactory::IAActionFactory(IAOptions options) : m_options(options) {}

std::unique_ptr<clang::FrontendAction> IAActionFactory::create() {
  return std::make_unique<IAFrontendAction>(m_editedFiles, m_options);
}

llvm::StringMap<std::string> const& IAActionFactory::getEditedFiles() const {
  return m_editedFiles;
}

} // namespace include_analyser
} // namespace clang