#include "../../../../../llvm/include/llvm/ADT/APFixedPoint.h"
#include "../../../../../llvm/include/llvm/ADT/APFloat.h"
#include "../../../../../llvm/include/llvm/ADT/DepthFirstIterator.h"
#include "../../../../../llvm/include/llvm/ADT/FunctionExtras.h"
#include "../../../../../llvm/include/llvm/ADT/ImmutableMap.h"
#include "../../../../../llvm/include/llvm/ADT/PagedVector.h"
#include "../../../../../llvm/include/llvm/ADT/SmallSet.h"
#include "../../../../../llvm/include/llvm/ADT/StringExtras.h"
#include "../../../../../llvm/include/llvm/ADT/TypeSwitch.h"
#include "../../../../../llvm/include/llvm/ADT/ilist.h"
#include "../../../../../llvm/include/llvm/Frontend/HLSL/HLSLRootSignature.h"
#include "../../../../../llvm/include/llvm/Support/AlignOf.h"
#include "../../../../../llvm/include/llvm/Support/AtomicOrdering.h"
#include "../../../../../llvm/include/llvm/Support/ConvertUTF.h"
#include "../../../../../llvm/include/llvm/Support/PrettyStackTrace.h"
#include "../../../../../llvm/include/llvm/Support/Registry.h"
#include "../../../../../llvm/include/llvm/Support/SaveAndRestore.h"
#include "../../../../../llvm/include/llvm/Target/TargetOptions.h"
#include "../../../../../llvm/include/llvm/TargetParser/AtomicScope.h"
#include "../../../../include/clang/AST/APNumericStorage.h"
#include "../../../../include/clang/AST/APValue.h"
#include "../../../../include/clang/AST/ASTContext.h"
#include "../../../../include/clang/AST/ASTContextAllocate.h"
#include "../../../../include/clang/AST/ASTDumperUtils.h"
#include "../../../../include/clang/AST/ASTFwd.h"
#include "../../../../include/clang/AST/ASTVector.h"
#include "../../../../include/clang/AST/Attr.h"
#include "../../../../include/clang/AST/CommentCommandTraits.h"
#include "../../../../include/clang/AST/ComputeDependence.h"
#include "../../../../include/clang/AST/DeclAccessPair.h"
#include "../../../../include/clang/AST/DeclCXX.h"
#include "../../../../include/clang/AST/DeclFriend.h"
#include "../../../../include/clang/AST/DeclGroup.h"
#include "../../../../include/clang/AST/DeclID.h"
#include "../../../../include/clang/AST/DeclObjCCommon.h"
#include "../../../../include/clang/AST/DeclTemplate.h"
#include "../../../../include/clang/AST/NestedNameSpecifier.h"
#include "../../../../include/clang/AST/OperationKinds.h"
#include "../../../../include/clang/AST/SelectorLocationsKind.h"
#include "../../../../include/clang/AST/StmtCXX.h"
#include "../../../../include/clang/AST/StmtIterator.h"
#include "../../../../include/clang/AST/TypeBase.h"
#include "../../../../include/clang/AST/TypeLoc.h"
#include "../../../../include/clang/Analysis/BodyFarm.h"
#include "../../../../include/clang/Analysis/CFG.h"
#include "../../../../include/clang/Analysis/CFGStmtMap.h"
#include "../../../../include/clang/Analysis/CodeInjector.h"
#include "../../../../include/clang/Analysis/DomainSpecific/ObjCNoReturn.h"
#include "../../../../include/clang/Analysis/PathDiagnostic.h"
#include "../../../../include/clang/Basic/AttributeCommonInfo.h"
#include "../../../../include/clang/Basic/BitmaskEnum.h"
#include "../../../../include/clang/Basic/BuiltinTraits.h"
#include "../../../../include/clang/Basic/Builtins.h"
#include "../../../../include/clang/Basic/CFProtectionOptions.h"
#include "../../../../include/clang/Basic/CapturedStmt.h"
#include "../../../../include/clang/Basic/CharInfo.h"
#include "../../../../include/clang/Basic/CustomizableOptional.h"
#include "../../../../include/clang/Basic/DiagnosticCategories.h"
#include "../../../../include/clang/Basic/DiagnosticIDs.h"
#include "../../../../include/clang/Basic/DiagnosticOptions.h"
#include "../../../../include/clang/Basic/FileManager.h"
#include "../../../../include/clang/Basic/IdentifierTable.h"
#include "../../../../include/clang/Basic/LangOptions.h"
#include "../../../../include/clang/Basic/LangStandard.h"
#include "../../../../include/clang/Basic/Module.h"
#include "../../../../include/clang/Basic/OpenMPKinds.h"
#include "../../../../include/clang/Basic/OperatorKinds.h"
#include "../../../../include/clang/Basic/OptionalUnsigned.h"
#include "../../../../include/clang/Basic/PragmaKinds.h"
#include "../../../../include/clang/Basic/Sanitizers.h"
#include "../../../../include/clang/Basic/SourceManager.h"
#include "../../../../include/clang/Basic/SyncScope.h"
#include "../../../../include/clang/Basic/TargetCXXABI.h"
#include "../../../../include/clang/Basic/TemplateKinds.h"
#include "../../../../include/clang/Basic/TokenKinds.h"
#include "../../../../include/clang/Basic/Version.h"
#include "../../../../include/clang/Lex/DependencyDirectivesScanner.h"
#include "../../../../include/clang/Lex/DirectoryLookup.h"
#include "../../../../include/clang/Lex/ExternalPreprocessorSource.h"
#include "../../../../include/clang/Lex/HeaderMap.h"
#include "../../../../include/clang/Lex/HeaderMapTypes.h"
#include "../../../../include/clang/Lex/HeaderSearch.h"
#include "../../../../include/clang/Lex/Lexer.h"
#include "../../../../include/clang/Lex/MacroInfo.h"
#include "../../../../include/clang/Lex/ModuleLoader.h"
#include "../../../../include/clang/Lex/ModuleMap.h"
#include "../../../../include/clang/Lex/ModuleMapFile.h"
#include "../../../../include/clang/Lex/MultipleIncludeOpt.h"
#include "../../../../include/clang/Lex/PPCallbacks.h"
#include "../../../../include/clang/Lex/PPDirectiveParameter.h"
#include "../../../../include/clang/Lex/PPEmbedParameters.h"
#include "../../../../include/clang/Lex/Pragma.h"
#include "../../../../include/clang/Lex/Preprocessor.h"
#include "../../../../include/clang/Lex/PreprocessorLexer.h"
#include "../../../../include/clang/Lex/TextEncoding.h"
#include "../../../../include/clang/Lex/Token.h"
#include "../../../../include/clang/Lex/TokenLexer.h"
#include "../../../../include/clang/StaticAnalyzer/Core/AnalyzerOptions.h"
#include "../../../../include/clang/StaticAnalyzer/Core/BugReporter/BugReporterVisitors.h"
#include "../../../../include/clang/StaticAnalyzer/Core/BugReporter/BugSuppression.h"
#include "../../../../include/clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "../../../../include/clang/StaticAnalyzer/Core/BugReporter/CommonBugCategories.h"
#include "../../../../include/clang/StaticAnalyzer/Core/Checker.h"
#include "../../../../include/clang/StaticAnalyzer/Core/CheckerManager.h"
#include "../../../../include/clang/StaticAnalyzer/Core/CheckerRegistryData.h"
#include "../../../../include/clang/StaticAnalyzer/Core/PathDiagnosticConsumers.h"
#include "../../../../include/clang/StaticAnalyzer/Core/PathSensitive/APSIntType.h"
#include "../../../../include/clang/StaticAnalyzer/Core/PathSensitive/AnalysisManager.h"
#include "../../../../include/clang/StaticAnalyzer/Core/PathSensitive/BasicValueFactory.h"
#include "../../../../include/clang/StaticAnalyzer/Core/PathSensitive/ConstraintManager.h"
#include "../../../../include/clang/StaticAnalyzer/Core/PathSensitive/CoreEngine.h"
#include "../../../../include/clang/StaticAnalyzer/Core/PathSensitive/DynamicTypeInfo.h"
#include "../../../../include/clang/StaticAnalyzer/Core/PathSensitive/Environment.h"
#include "../../../../include/clang/StaticAnalyzer/Core/PathSensitive/ExprEngine.h"
#include "../../../../include/clang/StaticAnalyzer/Core/PathSensitive/FunctionSummary.h"
#include "../../../../include/clang/StaticAnalyzer/Core/PathSensitive/MemRegion.h"
#include "../../../../include/clang/StaticAnalyzer/Core/PathSensitive/ProgramState.h"
#include "../../../../include/clang/StaticAnalyzer/Core/PathSensitive/ProgramStateTrait.h"
#include "../../../../include/clang/StaticAnalyzer/Core/PathSensitive/RangedConstraintManager.h"
#include "../../../../include/clang/StaticAnalyzer/Core/PathSensitive/SValBuilder.h"
#include "../../../../include/clang/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include "../../../../include/clang/StaticAnalyzer/Core/PathSensitive/SimpleConstraintManager.h"
#include "../../../../include/clang/StaticAnalyzer/Core/PathSensitive/Store.h"
#include "../../../../include/clang/StaticAnalyzer/Core/PathSensitive/StoreRef.h"
#include "../../../../include/clang/StaticAnalyzer/Core/PathSensitive/SymExpr.h"
#include "../../../../include/clang/StaticAnalyzer/Core/PathSensitive/SymbolManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Frontend/CheckerRegistry.h"

// This simple plugin is used by clang/test/Analysis/checker-plugins.c
// to test the use of a checker that is defined in a plugin.

using namespace clang;
using namespace ento;

namespace {
class MainCallChecker : public Checker<check::PreStmt<CallExpr>> {

  const BugType BT{this, "call to main", "example analyzer plugin"};

public:
  void checkPreStmt(const CallExpr *CE, CheckerContext &C) const;
};
} // end anonymous namespace

void MainCallChecker::checkPreStmt(const CallExpr *CE,
                                   CheckerContext &C) const {
  const Expr *Callee = CE->getCallee();
  const FunctionDecl *FD = C.getSVal(Callee).getAsFunctionDecl();

  if (!FD)
    return;

  // Get the name of the callee.
  IdentifierInfo *II = FD->getIdentifier();
  if (!II) // if no identifier, not a simple C function
    return;

  if (II->isStr("main")) {
    ExplodedNode *N = C.generateErrorNode();
    if (!N)
      return;

    auto report =
        std::make_unique<PathSensitiveBugReport>(BT, BT.getDescription(), N);
    report->addRange(Callee->getSourceRange());
    C.emitReport(std::move(report));
  }
}

// Register plugin!
extern "C" void clang_registerCheckers(CheckerRegistry &Registry) {
  Registry.addChecker<MainCallChecker>("example.MainCallChecker",
                                       "Example Description");
}

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;
