#ifndef INCLUDE_ANALYSER_INCLUSION_COLLECTOR_H
#define INCLUDE_ANALYSER_INCLUSION_COLLECTOR_H

#include <list>
#include <filesystem>

#include "clang/Lex/PPCallbacks.h"
#include "clang/Frontend/CompilerInstance.h"

#include "Graph.h"

class InclusionCollector : public clang::PPCallbacks {
public:
  using PathType = std::filesystem::path;

  struct FileData {
    PathType const* m_path;
  };

  struct MacroData {
    clang::SourceLocation m_defineLoc;
    clang::SourceLocation m_expansionLoc;
  };

  using GraphType = Graph<FileData>;
  using GraphIterator = GraphType::NodeIterator;
  using InclusionMap = std::map<PathType, GraphIterator>;
  using IncMapIterator = InclusionMap::iterator;
  using ConstIncMapIterator = InclusionMap::const_iterator;

  InclusionCollector(clang::CompilerInstance& compiler);

  void InclusionDirective(
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
  ) override;

  ConstIncMapIterator beginInclusions() const;
  ConstIncMapIterator cbeginInclusions() const;
  ConstIncMapIterator endInclusions() const;
  ConstIncMapIterator cendInclusions() const;

  void removeInclusion(ConstIncMapIterator iter);

  InclusionMap const& getInclusions() const;

  GraphType const& getGraph() const;

  using MacroList = std::list<MacroData>;
  using MacroIterator = MacroList::iterator;
  using ConstMacroIterator = MacroList::const_iterator;

  void MacroExpands(
    clang::Token const& nameToken,
    clang::MacroDefinition const& definition,
    clang::SourceRange srcRange,
    clang::MacroArgs const* args
  ) override;

  ConstMacroIterator beginMacros() const;
  ConstMacroIterator cbeginMacros() const;
  ConstMacroIterator endMacros() const;
  ConstMacroIterator cendMacros() const;

  MacroList const& getMacros() const;

private:
  clang::CompilerInstance* m_compiler;
  GraphType m_includeGraph;
  InclusionMap m_inclusions;
  std::list<MacroData> m_macros;
};

#endif // INCLUDE_ANALYSER_INCLUSION_COLLECTOR_H