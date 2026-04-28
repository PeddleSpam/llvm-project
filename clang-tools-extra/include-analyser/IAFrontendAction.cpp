#include "IAFrontendAction.h"

IAFrontendAction::IAFrontendAction(llvm::raw_ostream& output) : 
  m_output(&output), m_collector(nullptr), m_consumer(nullptr), 
  m_compiler(nullptr)
{}

std::unique_ptr<clang::ASTConsumer> IAFrontendAction::CreateASTConsumer(
  clang::CompilerInstance& compiler, llvm::StringRef file) {

  auto* context = &compiler.getASTContext();
  auto consumer = std::make_unique<IAConsumer>(context);
  m_consumer = consumer.get();
  return std::move(consumer);
}

bool IAFrontendAction::BeginSourceFileAction(
    clang::CompilerInstance &compiler) {
    
  m_compiler = &compiler;
  auto collector = std::make_unique<InclusionCollector>(compiler);
  m_collector = collector.get();
  compiler.getPreprocessor().addPPCallbacks(std::move(collector));

  return true;
}

void IAFrontendAction::EndSourceFileAction() { 
  using PathType = InclusionCollector::PathType;

  // Get include directives in main file.
  using FileData = InclusionCollector::FileData;
  auto& srcMgr = m_compiler->getSourceManager();
  auto mainFileID = srcMgr.getMainFileID();
  auto mainFileLoc = srcMgr.getLocForStartOfFile(mainFileID);
  auto mainFilePath = PathType(srcMgr.getFilename(mainFileLoc).str());

  auto& inclusions = m_collector->getInclusions();
  auto mainIter = inclusions.find(mainFilePath);
  if (mainIter != inclusions.end()) {
    *m_output << "Inclusions in main file:\n";
    for (auto& child : mainIter->second->getChildren()) {
      auto* path = child->getData().m_path;
      *m_output << "    > " << path->generic_string() << "\n";
    }
    *m_output << "\n";
  }
  else {
    *m_output << "Could not find main file.\n";
  }

  // Find the set of include files that contain type, value, or function
  // declarations used by main source file.
  auto& visitor = m_consumer->getVisitor();

  for (auto& typeLoc : visitor.getTypeLocs()) {
    if (srcMgr.getFileID(typeLoc.getBeginLoc()) == mainFileID) {
      if (auto* type = typeLoc.getType().getTypePtrOrNull()) {
        if (auto* record = type->getAsRecordDecl()) {
          auto srcLoc = record->getLocation();
          auto& srcMgr = m_compiler->getSourceManager();
          auto fileID = srcMgr.getFileID(srcLoc);
          if (fileID != srcMgr.getMainFileID()) {
            auto path = PathType(srcMgr.getFilename(srcLoc).str());
            m_optimalIncludes.insert(path);
          }
        }
      }
    }
  }

  for (auto& expr : visitor.getExprs()) {
    if (srcMgr.getFileID(expr->getEndLoc()) == mainFileID) {

      *m_output << "========\n";
      expr->dump(*m_output, m_compiler->getASTContext());
      *m_output << "========\n";

      if (auto* declRef = llvm::dyn_cast<clang::DeclRefExpr>(expr)) {
        auto srcLoc = declRef->getDecl()->getLocation();
        auto fileID = srcMgr.getFileID(srcLoc);
        if (fileID != mainFileID) {
          auto path = PathType(srcMgr.getFilename(srcLoc).str());
          m_optimalIncludes.insert(path);
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
          m_optimalIncludes.insert(path);
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
        m_optimalIncludes.insert(path);
      }
    }
  }

  // Remove any include directives that were not found.
  auto removals = std::list<InclusionCollector::ConstIncMapIterator>();

  for (auto iter = m_collector->beginInclusions(); 
            iter != m_collector->endInclusions(); ++iter) {
    if (m_optimalIncludes.find(iter->first) == m_optimalIncludes.end()) {
      removals.push_back(iter);
    }
  }

  for (auto& iter : removals) {
    m_collector->removeInclusion(iter);
  }

  // Collect all includes that have no parents.
  auto collected = std::set<PathType>();

  for (auto& inc : m_collector->getInclusions()) {
    if (inc.second->getNumParents() == 0u ||
       (inc.second->getNumParents() == 1u &&
        inc.first == mainFilePath)) {
      collected.insert(inc.first);
    }
  }

  // Output used include files.
  *m_output << "Inclusions required by main file:\n";
  for (auto& path : collected) {
    *m_output << "    > " << path.generic_string() << "\n";
  }
  *m_output << "\n";
}