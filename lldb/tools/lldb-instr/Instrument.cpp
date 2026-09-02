#include "../../../clang/include/clang-c/BuildSystem.h"
#include "../../../clang/include/clang-c/CXDiagnostic.h"
#include "../../../clang/include/clang-c/CXErrorCode.h"
#include "../../../clang/include/clang-c/CXFile.h"
#include "../../../clang/include/clang-c/CXSourceLocation.h"
#include "../../../clang/include/clang-c/Index.h"
#include "../../../clang/include/clang/APINotes/APINotesOptions.h"
#include "../../../clang/include/clang/AST/APNumericStorage.h"
#include "../../../clang/include/clang/AST/APValue.h"
#include "../../../clang/include/clang/AST/ASTConsumer.h"
#include "../../../clang/include/clang/AST/ASTContextAllocate.h"
#include "../../../clang/include/clang/AST/ASTDumperUtils.h"
#include "../../../clang/include/clang/AST/ASTFwd.h"
#include "../../../clang/include/clang/AST/ASTVector.h"
#include "../../../clang/include/clang/AST/Attr.h"
#include "../../../clang/include/clang/AST/CanonicalType.h"
#include "../../../clang/include/clang/AST/CommentCommandTraits.h"
#include "../../../clang/include/clang/AST/DeclAccessPair.h"
#include "../../../clang/include/clang/AST/DeclCXX.h"
#include "../../../clang/include/clang/AST/DeclGroup.h"
#include "../../../clang/include/clang/AST/DeclObjCCommon.h"
#include "../../../clang/include/clang/AST/ExprCXX.h"
#include "../../../clang/include/clang/AST/NestedNameSpecifierBase.h"
#include "../../../clang/include/clang/AST/OpenACCClause.h"
#include "../../../clang/include/clang/AST/OperationKinds.h"
#include "../../../clang/include/clang/AST/PrettyPrinter.h"
#include "../../../clang/include/clang/AST/RawCommentList.h"
#include "../../../clang/include/clang/AST/RecursiveASTVisitor.h"
#include "../../../clang/include/clang/AST/SYCLKernelInfo.h"
#include "../../../clang/include/clang/AST/SelectorLocationsKind.h"
#include "../../../clang/include/clang/AST/StmtVisitor.h"
#include "../../../clang/include/clang/AST/TypeBase.h"
#include "../../../clang/include/clang/AST/TypeOrdering.h"
#include "../../../clang/include/clang/Analysis/Analyses/LifetimeSafety/LifetimeStats.h"
#include "../../../clang/include/clang/Basic/AttrSubjectMatchRules.h"
#include "../../../clang/include/clang/Basic/AttributeCommonInfo.h"
#include "../../../clang/include/clang/Basic/Builtins.h"
#include "../../../clang/include/clang/Basic/CFProtectionOptions.h"
#include "../../../clang/include/clang/Basic/CharInfo.h"
#include "../../../clang/include/clang/Basic/CodeGenOptions.h"
#include "../../../clang/include/clang/Basic/CustomizableOptional.h"
#include "../../../clang/include/clang/Basic/Diagnostic.h"
#include "../../../clang/include/clang/Basic/DiagnosticCategories.h"
#include "../../../clang/include/clang/Basic/DiagnosticIDs.h"
#include "../../../clang/include/clang/Basic/DiagnosticOptions.h"
#include "../../../clang/include/clang/Basic/DiagnosticSema.h"
#include "../../../clang/include/clang/Basic/DirectoryEntry.h"
#include "../../../clang/include/clang/Basic/FileEntry.h"
#include "../../../clang/include/clang/Basic/FileManager.h"
#include "../../../clang/include/clang/Basic/FileSystemOptions.h"
#include "../../../clang/include/clang/Basic/HeaderInclude.h"
#include "../../../clang/include/clang/Basic/LangOptions.h"
#include "../../../clang/include/clang/Basic/LangStandard.h"
#include "../../../clang/include/clang/Basic/MakeSupport.h"
#include "../../../clang/include/clang/Basic/OffloadArch.h"
#include "../../../clang/include/clang/Basic/OpenMPKinds.h"
#include "../../../clang/include/clang/Basic/OperatorKinds.h"
#include "../../../clang/include/clang/Basic/ParsedAttrInfo.h"
#include "../../../clang/include/clang/Basic/Sanitizers.h"
#include "../../../clang/include/clang/Basic/SourceManager.h"
#include "../../../clang/include/clang/Basic/SyncScope.h"
#include "../../../clang/include/clang/Basic/TargetCXXABI.h"
#include "../../../clang/include/clang/Basic/TargetInfo.h"
#include "../../../clang/include/clang/Basic/TokenKinds.h"
#include "../../../clang/include/clang/Frontend/ASTUnit.h"
#include "../../../clang/include/clang/Frontend/CommandLineSourceLoc.h"
#include "../../../clang/include/clang/Frontend/CompilerInstance.h"
#include "../../../clang/include/clang/Frontend/CompilerInvocation.h"
#include "../../../clang/include/clang/Frontend/DependencyOutputOptions.h"
#include "../../../clang/include/clang/Frontend/FrontendAction.h"
#include "../../../clang/include/clang/Frontend/FrontendOptions.h"
#include "../../../clang/include/clang/Frontend/MigratorOptions.h"
#include "../../../clang/include/clang/Frontend/PrecompiledPreamble.h"
#include "../../../clang/include/clang/Frontend/PreprocessorOutputOptions.h"
#include "../../../clang/include/clang/Frontend/StandaloneDiagnostic.h"
#include "../../../clang/include/clang/Frontend/Utils.h"
#include "../../../clang/include/clang/Lex/DependencyDirectivesScanner.h"
#include "../../../clang/include/clang/Lex/DirectoryLookup.h"
#include "../../../clang/include/clang/Lex/ExternalPreprocessorSource.h"
#include "../../../clang/include/clang/Lex/HeaderMap.h"
#include "../../../clang/include/clang/Lex/HeaderMapTypes.h"
#include "../../../clang/include/clang/Lex/HeaderSearch.h"
#include "../../../clang/include/clang/Lex/HeaderSearchOptions.h"
#include "../../../clang/include/clang/Lex/Lexer.h"
#include "../../../clang/include/clang/Lex/MacroBase.h"
#include "../../../clang/include/clang/Lex/MacroInfo.h"
#include "../../../clang/include/clang/Lex/ModuleLoader.h"
#include "../../../clang/include/clang/Lex/ModuleMap.h"
#include "../../../clang/include/clang/Lex/ModuleMapFile.h"
#include "../../../clang/include/clang/Lex/MultipleIncludeOpt.h"
#include "../../../clang/include/clang/Lex/PPCallbacks.h"
#include "../../../clang/include/clang/Lex/PPDirectiveParameter.h"
#include "../../../clang/include/clang/Lex/PPEmbedParameters.h"
#include "../../../clang/include/clang/Lex/Pragma.h"
#include "../../../clang/include/clang/Lex/PreprocessingRecord.h"
#include "../../../clang/include/clang/Lex/Preprocessor.h"
#include "../../../clang/include/clang/Lex/PreprocessorLexer.h"
#include "../../../clang/include/clang/Lex/TextEncoding.h"
#include "../../../clang/include/clang/Lex/Token.h"
#include "../../../clang/include/clang/Lex/TokenLexer.h"
#include "../../../clang/include/clang/Options/OptionUtils.h"
#include "../../../clang/include/clang/Sema/CodeCompleteConsumer.h"
#include "../../../clang/include/clang/Sema/CodeCompleteOptions.h"
#include "../../../clang/include/clang/Sema/ScopeInfo.h"
#include "../../../clang/include/clang/Sema/Sema.h"
#include "../../../clang/include/clang/Serialization/ASTBitCodes.h"
#include "../../../clang/include/clang/Serialization/ASTWriter.h"
#include "../../../clang/include/clang/Serialization/ModuleFileExtension.h"
#include "../../../clang/include/clang/Tooling/ArgumentsAdjusters.h"
#include "../../../clang/include/clang/Tooling/CompilationDatabase.h"
#include "../../../llvm/include/llvm/ADT/APFixedPoint.h"
#include "../../../llvm/include/llvm/ADT/CachedHashString.h"
#include "../../../llvm/include/llvm/ADT/PagedVector.h"
#include "../../../llvm/include/llvm/ADT/RewriteBuffer.h"
#include "../../../llvm/include/llvm/ADT/ScopeExit.h"
#include "../../../llvm/include/llvm/Frontend/HLSL/HLSLRootSignature.h"
#include "../../../llvm/include/llvm/Frontend/OpenMP/OMPAssume.h"
#include "../../../llvm/include/llvm/Frontend/OpenMP/OMPContext.h"
#include "../../../llvm/include/llvm/Option/Option.h"
#include "../../../llvm/include/llvm/Support/AlignOf.h"
#include "../../../llvm/include/llvm/Support/AtomicOrdering.h"
#include "../../../llvm/include/llvm/Support/BuryPointer.h"
#include "../../../llvm/include/llvm/Support/CommandLine.h"
#include "../../../llvm/include/llvm/Support/FileCollector.h"
#include "../../../llvm/include/llvm/Support/PrettyStackTrace.h"
#include "../../../llvm/include/llvm/Support/Registry.h"
#include "../../../llvm/include/llvm/Support/TypeSize.h"
#include "../../../llvm/include/llvm/Support/VirtualOutputBackend.h"
#include "../../../llvm/include/llvm/TargetParser/AtomicScope.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/CodeGen/ObjectFilePCHContainerWriter.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Serialization/ObjectFilePCHContainerReader.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"


#include <sstream>
#include <string>

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;

static llvm::cl::OptionCategory InstrCategory("LLDB Instrumentation Generator");

class SBVisitor : public RecursiveASTVisitor<SBVisitor> {
public:
  SBVisitor(Rewriter &R, ASTContext &Context)
      : MyRewriter(R), Context(Context) {}

  bool VisitCXXMethodDecl(CXXMethodDecl *Decl) {
    // Not all decls should be registered. Please refer to that method's
    // comment for details.
    if (ShouldSkip(Decl))
      return false;

    // Print 'bool' instead of '_Bool'.
    PrintingPolicy Policy(Context.getLangOpts());
    Policy.Bool = true;

    // Collect the functions parameter types and names.
    std::vector<std::string> ParamNames;
    if (!Decl->isStatic())
      ParamNames.push_back("this");
    for (auto *P : Decl->parameters())
      ParamNames.push_back(P->getNameAsString());

    // Construct the macros.
    std::string Buffer;
    llvm::raw_string_ostream Macro(Buffer);
    if (ParamNames.empty()) {
      Macro << "LLDB_INSTRUMENT()";
    } else {
      Macro << "LLDB_INSTRUMENT_VA(" << llvm::join(ParamNames, ", ") << ")";
    }

    Stmt *Body = Decl->getBody();
    for (auto &C : Body->children()) {
      if (C->getBeginLoc().isMacroID()) {
        CharSourceRange Range =
            MyRewriter.getSourceMgr().getExpansionRange(C->getSourceRange());
        MyRewriter.ReplaceText(Range, Buffer);
      } else {
        Macro << ";";
        SourceLocation InsertLoc = Lexer::getLocForEndOfToken(
            Body->getBeginLoc(), 0, MyRewriter.getSourceMgr(),
            MyRewriter.getLangOpts());
        MyRewriter.InsertTextAfter(InsertLoc, Buffer);
      }
      break;
    }

    return true;
  }

private:
  /// Determine whether we need to consider the given CXXMethodDecl.
  ///
  /// Currently we skip the following cases:
  ///  1. Decls outside the main source file,
  ///  2. Decls that are only present in the source file,
  ///  3. Decls that are not definitions,
  ///  4. Non-public methods,
  ///  5. Variadic methods.
  ///  6. Destructors.
  bool ShouldSkip(CXXMethodDecl *Decl) {
    // Skip anything outside the main file.
    if (!MyRewriter.getSourceMgr().isInMainFile(Decl->getBeginLoc()))
      return true;

    // Skip if the canonical decl in the current decl. It means that the method
    // is declared in the implementation and is therefore not exposed as part
    // of the API.
    if (Decl == Decl->getCanonicalDecl())
      return true;

    // Skip decls that have no body, i.e. are just declarations.
    Stmt *Body = Decl->getBody();
    if (!Body)
      return true;

    // Skip non-public methods.
    AccessSpecifier AS = Decl->getAccess();
    if (AS != AccessSpecifier::AS_public)
      return true;

    // Skip variadic methods.
    if (Decl->isVariadic())
      return true;

    // Skip destructors.
    if (isa<CXXDestructorDecl>(Decl))
      return true;

    return false;
  }

  Rewriter &MyRewriter;
  ASTContext &Context;
};

class SBConsumer : public ASTConsumer {
public:
  SBConsumer(Rewriter &R, ASTContext &Context) : Visitor(R, Context) {}

  // Override the method that gets called for each parsed top-level
  // declaration.
  bool HandleTopLevelDecl(DeclGroupRef DR) override {
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b) {
      Visitor.TraverseDecl(*b);
    }
    return true;
  }

private:
  SBVisitor Visitor;
};

class SBAction : public ASTFrontendAction {
public:
  SBAction() = default;

  bool BeginSourceFileAction(CompilerInstance &CI) override { return true; }

  void EndSourceFileAction() override { MyRewriter.overwriteChangedFiles(); }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef File) override {
    MyRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return std::make_unique<SBConsumer>(MyRewriter, CI.getASTContext());
  }

private:
  Rewriter MyRewriter;
};

int main(int argc, const char **argv) {
  auto ExpectedParser = CommonOptionsParser::create(
      argc, argv, InstrCategory, llvm::cl::OneOrMore,
      "Utility for generating the macros for LLDB's "
      "instrumentation framework.");
  if (!ExpectedParser) {
    llvm::errs() << ExpectedParser.takeError();
    return 1;
  }
  CommonOptionsParser &OP = ExpectedParser.get();

  auto PCHOpts = std::make_shared<PCHContainerOperations>();
  PCHOpts->registerWriter(std::make_unique<ObjectFilePCHContainerWriter>());
  PCHOpts->registerReader(std::make_unique<ObjectFilePCHContainerReader>());

  ClangTool T(OP.getCompilations(), OP.getSourcePathList(), PCHOpts);
  return T.run(newFrontendActionFactory<SBAction>().get());
}
