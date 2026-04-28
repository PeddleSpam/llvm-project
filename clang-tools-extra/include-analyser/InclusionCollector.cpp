#include "InclusionCollector.h"

#include <clang/Lex/Token.h>
#include <clang/Lex/MacroInfo.h>

InclusionCollector::InclusionCollector(clang::CompilerInstance& compiler) : 
  m_compiler(&compiler)
{}

void InclusionCollector::InclusionDirective(
    clang::SourceLocation hashLoc,
    clang::Token const& includeTok,
    clang::StringRef fileName,
    bool isAngled,
    clang::CharSourceRange filenameRange,
    clang::OptionalFileEntryRef file,
    clang::StringRef searchPath,
    clang::StringRef relativePath,
    clang::Module const* suggestedModule,
    bool moduleImported,
    clang::SrcMgr::CharacteristicKind fileType
  ) {

  auto& srcMgr = m_compiler->getSourceManager();

  // Ensure the source file has an entry in the graph.
  auto srcFilePath = PathType(std::string(srcMgr.getFilename(hashLoc)));
  auto srcEntry = m_inclusions.insert({srcFilePath, m_includeGraph.endNodes()});
  if (srcEntry.second) {
    auto* pathPtr = &(srcEntry.first->first);
    m_inclusions[srcFilePath] = m_includeGraph.addNode({pathPtr});
  }
  auto srcFileIter = m_inclusions[srcFilePath];

  // Ensure the included file has an entry in the graph.
  auto incFilePath = PathType(std::string(searchPath));
  incFilePath.append(std::string(fileName));
  auto incEntry = m_inclusions.insert({incFilePath, m_includeGraph.endNodes()});
  if (incEntry.second) {
    auto* pathPtr = &(incEntry.first->first);
    m_inclusions[incFilePath] = m_includeGraph.addNode({pathPtr});
  }
  auto incFileIter = m_inclusions[incFilePath];
  
  // Add parent-child connections.
  srcFileIter->addChild(incFileIter);
  incFileIter->addParent(srcFileIter);
}

InclusionCollector::ConstIncMapIterator
InclusionCollector::beginInclusions() const {
  return m_inclusions.begin();
}

InclusionCollector::ConstIncMapIterator
InclusionCollector::cbeginInclusions() const {
  return m_inclusions.begin();
}

InclusionCollector::ConstIncMapIterator
InclusionCollector::endInclusions() const {
  return m_inclusions.end();
}

InclusionCollector::ConstIncMapIterator
InclusionCollector::cendInclusions() const {
  return m_inclusions.end();
}

void InclusionCollector::removeInclusion(ConstIncMapIterator iter) {
  m_includeGraph.removeNode(iter->second);
  m_inclusions.erase(iter);
}

InclusionCollector::InclusionMap const& InclusionCollector::getInclusions() const {
  return m_inclusions;
}

InclusionCollector::GraphType const& InclusionCollector::getGraph() const {
  return m_includeGraph;
}

void InclusionCollector::MacroExpands(
    clang::Token const& token,
    clang::MacroDefinition const& definition,
    clang::SourceRange srcRange,
    clang::MacroArgs const* args
  ) {
  auto* macroInfo = definition.getMacroInfo();
  m_macros.push_back({macroInfo->getDefinitionLoc(), srcRange.getBegin()});
}

InclusionCollector::ConstMacroIterator
InclusionCollector::beginMacros() const {
  return m_macros.begin();
}

InclusionCollector::ConstMacroIterator
InclusionCollector::cbeginMacros() const {
  return m_macros.begin();
}

InclusionCollector::ConstMacroIterator
InclusionCollector::endMacros() const {
  return m_macros.end();
}

InclusionCollector::ConstMacroIterator
InclusionCollector::cendMacros() const {
  return m_macros.end();
}

InclusionCollector::MacroList const& InclusionCollector::getMacros() const {
  return m_macros;
}