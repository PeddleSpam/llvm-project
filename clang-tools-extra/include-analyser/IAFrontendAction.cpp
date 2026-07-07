#include "IAFrontendAction.h"
#include "clang/Tooling/Core/Replacement.h"
#include "clang/Format/Format.h"
#include "llvm/Support/Error.h"

namespace clang {
namespace include_analyser {

IAFrontendAction::IAFrontendAction(
    llvm::StringMap<std::string>& editedFiles,
    IAOptions& options
) : 
  m_options(&options), m_consumer(nullptr), m_collector(nullptr), 
  m_editedFiles(&editedFiles)
  {}

std::unique_ptr<clang::ASTConsumer> IAFrontendAction::CreateASTConsumer(
    clang::CompilerInstance& compiler, llvm::StringRef file) {

  auto* context = &compiler.getASTContext();
  auto consumer = std::make_unique<IAConsumer>(context);
  m_consumer = consumer.get();
  return std::move(consumer);
}

bool IAFrontendAction::BeginInvocation(clang::CompilerInstance &compiler) {
  // Disable diagnostics that won't affect analysis.
  compiler.getLangOpts().ModulesDeclUse = false;
  compiler.getLangOpts().ModulesStrictDeclUse = false;
  return true;
}

bool IAFrontendAction::BeginSourceFileAction(
    clang::CompilerInstance &compiler) {
    
  auto collector = std::make_unique<InclusionCollector>(compiler);
  m_collector = collector.get();
  compiler.getPreprocessor().addPPCallbacks(std::move(collector));

  return true;
}

void IAFrontendAction::ExecuteAction() {
  // Disable warnings.
  auto &diags = getCompilerInstance().getDiagnostics();
  diags.setEnableAllWarnings(false);
  diags.setSeverityForAll(clang::diag::Flavor::WarningOrError,
                          clang::diag::Severity::Ignored);
  ASTFrontendAction::ExecuteAction();
}

void IAFrontendAction::EndSourceFileAction() { 
  using PathType = InclusionCollector::PathType;

  // Check for compilation errors.
  auto &srcMgr = getCompilerInstance().getSourceManager();

  //if (srcMgr.getDiagnostics().hasUncompilableErrorOccurred()) {
  //  llvm::errs() << "Skipping file " << getCurrentFile()
  //               << " due to compiler errors. include-analyser expects to "
  //                  "work on compilable source code.\n";
  //  return;
  //}

  // Find the set of include files that contain type, value, or function
  // declarations used by the main source file.
  auto& visitor = m_consumer->getVisitor();
  auto mainFileID = srcMgr.getMainFileID();
  auto mainFileLoc = srcMgr.getLocForStartOfFile(mainFileID);
  auto mainFilePath = PathType(srcMgr.getFilename(mainFileLoc).str());
  auto foundIncludes = std::set<PathType>();

  for (auto& typeLoc : visitor.getTypeLocs()) {
    if (srcMgr.getFileID(typeLoc.getBeginLoc()) == mainFileID) {
      if (auto* type = typeLoc.getType().getTypePtrOrNull()) {
        if (auto* record = type->getAsRecordDecl()) {
          auto srcLoc = record->getLocation();
          auto fileID = srcMgr.getFileID(srcLoc);
          if (fileID != mainFileID) {
            auto path = PathType(srcMgr.getFilename(srcLoc).str());
            foundIncludes.insert(path);
          }
        }
      }
    }
  }

  for (auto& expr : visitor.getExprs()) {
    if (srcMgr.getFileID(expr->getEndLoc()) == mainFileID) {
      if (auto* declRef = llvm::dyn_cast<clang::DeclRefExpr>(expr)) {
        auto srcLoc = declRef->getDecl()->getLocation();
        auto fileID = srcMgr.getFileID(srcLoc);
        if (fileID != mainFileID) {
          auto path = PathType(srcMgr.getFilename(srcLoc).str());
          foundIncludes.insert(path);
        }
      }
    }
  }

  for (auto& callExpr : visitor.getCallExprs()) {
    if (srcMgr.getFileID(callExpr->getBeginLoc()) == mainFileID) {
      if (auto* calleeDecl = callExpr->getCalleeDecl()) {
        auto srcLoc = calleeDecl->getLocation();
        auto fileID = srcMgr.getFileID(srcLoc);
        if (fileID != mainFileID) {
          auto path = PathType(srcMgr.getFilename(srcLoc).str());
          foundIncludes.insert(path);
        }
      }
    }
  }

  // Get macro expansions in main file.
  for (auto& macro : m_collector->getMacros()) {
    if (srcMgr.getFileID(macro.m_expansionLoc) == mainFileID) {
      auto fileID = srcMgr.getFileID(macro.m_defineLoc);
      if (fileID != mainFileID) {
        auto path = PathType(srcMgr.getFilename(macro.m_defineLoc).str());
        foundIncludes.insert(path);
      }
    }
  }

  // Collect missing includes.
  auto &inclusions = m_collector->getInclusions();
  auto missing = std::set<PathType>();

  if (!m_options->m_disableInsert) {
    for (auto iter = foundIncludes.begin(); iter != foundIncludes.end();
         ++iter) {
      if (inclusions.find(*iter) == inclusions.end()) {
        missing.insert(*iter);
      }
    }
  }

  // Collect unused includes.
  auto unused = std::set<PathType>();

  if (!m_options->m_disableRemove) {
    for (auto iter = inclusions.begin(); iter != inclusions.end(); ++iter) {
      if (iter->first != mainFilePath) {
        // Collect includes that were not found.
        if (foundIncludes.find(iter->first) == foundIncludes.end()) {
          unused.insert(iter->first);
        }

        // Collect includes that are not roots.
        if ((iter->second->getNumParents() > 1u) ||
            (iter->second->getNumParents() == 1u &&
             iter->first == mainFilePath)) {
          unused.insert(iter->first);
        }
      }
    }
  }

  // Encode insertions and deletions.
  using Replacement = tooling::Replacement;
  auto repl = tooling::Replacements();
  auto mainFileName = mainFilePath.generic_string();
  auto& filter = m_options->m_headerFilter;

  for (auto& incPath : missing) {
    if (!incPath.empty() && !filter(incPath.generic_string())) {
      auto outStr =
          std::string("#include \"") + incPath.generic_string() + "\"";
      repl.add(Replacement(mainFileName, UINT_MAX, 1u, outStr));
    }
  }

  for (auto &incPath : unused) {
    auto iter = inclusions.find(incPath);
    if (iter != inclusions.end()) {
      auto& data = iter->second->getData();
      auto outStr = data.m_path->generic_string();
      if (outStr.empty())
        outStr = data.m_path->generic_string();
      if (!outStr.empty() && !filter(outStr)) {
        if (false) {//data.m_isAngled) { // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
          outStr = "#include <" + outStr + ">";
        } else {
          outStr = "#include \"" + outStr + "\"";
        }
        repl.add(Replacement(mainFileName, UINT_MAX, 0u, outStr));
      }
    }
  }

  // Get style of main source file.
  auto style = format::getStyle(format::DefaultFormatStyle, mainFileName,
                                format::DefaultFallbackStyle);
  if (!style || !style->isCpp()) {
    llvm::consumeError(style.takeError());
    style = format::getLLVMStyle();
  }

  // Cleanup edits.
  auto code = srcMgr.getBufferData(mainFileID);
  auto positioned = 
      cantFail(format::cleanupAroundReplacements(code, repl, *style));
  auto result = cantFail(tooling::applyAllReplacements(code, positioned));
  m_editedFiles->insert({mainFileName, result});
}

} // namespace include_analyser
} // namespace clang