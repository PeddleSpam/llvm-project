//===- TestTransformation.cpp ---------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// A transformation used only by lit tests for the source-edit-generation
// pipeline. It walks every function in the main source file, emits a
// zero-length `/*T*/` comment at the function body's start, and adds one
// `test-touches-function` note per visited function. Its level is always
// `Note` — the goal is to exercise the framework's plumbing, not to
// produce meaningful findings. The level rises to `Warning` when the
// input WPASuite's id table is non-empty, giving lit tests a knob to
// confirm the suite is read at all without depending on namespace
// matching.
//
//===----------------------------------------------------------------------===//

#include "../../../../../../../llvm/include/llvm/ADT/APFixedPoint.h"
#include "../../../../../../../llvm/include/llvm/ADT/APFloat.h"
#include "../../../../../../../llvm/include/llvm/ADT/PagedVector.h"
#include "../../../../../../../llvm/include/llvm/ADT/SetVector.h"
#include "../../../../../../../llvm/include/llvm/ADT/SmallPtrSet.h"
#include "../../../../../../../llvm/include/llvm/ADT/StringSet.h"
#include "../../../../../../../llvm/include/llvm/Frontend/HLSL/HLSLRootSignature.h"
#include "../../../../../../../llvm/include/llvm/Support/AtomicOrdering.h"
#include "../../../../../../../llvm/include/llvm/Support/PrettyStackTrace.h"
#include "../../../../../../../llvm/include/llvm/Support/Registry.h"
#include "../../../../../../../llvm/include/llvm/Support/TypeSize.h"
#include "../../../../../../../llvm/include/llvm/Target/TargetOptions.h"
#include "../../../../../../../llvm/include/llvm/TargetParser/AtomicScope.h"
#include "../../../../../../include/clang/AST/APNumericStorage.h"
#include "../../../../../../include/clang/AST/APValue.h"
#include "../../../../../../include/clang/AST/ASTConsumer.h"
#include "../../../../../../include/clang/AST/ASTContextAllocate.h"
#include "../../../../../../include/clang/AST/ASTDumperUtils.h"
#include "../../../../../../include/clang/AST/ASTVector.h"
#include "../../../../../../include/clang/AST/Attr.h"
#include "../../../../../../include/clang/AST/CanonicalType.h"
#include "../../../../../../include/clang/AST/CommentCommandTraits.h"
#include "../../../../../../include/clang/AST/DeclAccessPair.h"
#include "../../../../../../include/clang/AST/DeclCXX.h"
#include "../../../../../../include/clang/AST/DeclGroup.h"
#include "../../../../../../include/clang/AST/DeclID.h"
#include "../../../../../../include/clang/AST/DeclObjCCommon.h"
#include "../../../../../../include/clang/AST/DeclTemplate.h"
#include "../../../../../../include/clang/AST/ExprCXX.h"
#include "../../../../../../include/clang/AST/ExprConcepts.h"
#include "../../../../../../include/clang/AST/ExprObjC.h"
#include "../../../../../../include/clang/AST/ExternalASTSource.h"
#include "../../../../../../include/clang/AST/OpenACCClause.h"
#include "../../../../../../include/clang/AST/OpenMPClause.h"
#include "../../../../../../include/clang/AST/OperationKinds.h"
#include "../../../../../../include/clang/AST/PrettyPrinter.h"
#include "../../../../../../include/clang/AST/RawCommentList.h"
#include "../../../../../../include/clang/AST/RecursiveASTVisitor.h"
#include "../../../../../../include/clang/AST/SYCLKernelInfo.h"
#include "../../../../../../include/clang/AST/StmtOpenACC.h"
#include "../../../../../../include/clang/AST/TypeBase.h"
#include "../../../../../../include/clang/AST/TypeOrdering.h"
#include "../../../../../../include/clang/Basic/AttrKinds.h"
#include "../../../../../../include/clang/Basic/AttributeCommonInfo.h"
#include "../../../../../../include/clang/Basic/BitmaskEnum.h"
#include "../../../../../../include/clang/Basic/Builtins.h"
#include "../../../../../../include/clang/Basic/CFProtectionOptions.h"
#include "../../../../../../include/clang/Basic/CapturedStmt.h"
#include "../../../../../../include/clang/Basic/CharInfo.h"
#include "../../../../../../include/clang/Basic/CustomizableOptional.h"
#include "../../../../../../include/clang/Basic/Diagnostic.h"
#include "../../../../../../include/clang/Basic/DiagnosticCategories.h"
#include "../../../../../../include/clang/Basic/DiagnosticIDs.h"
#include "../../../../../../include/clang/Basic/DiagnosticOptions.h"
#include "../../../../../../include/clang/Basic/FileManager.h"
#include "../../../../../../include/clang/Basic/IdentifierTable.h"
#include "../../../../../../include/clang/Basic/LangOptions.h"
#include "../../../../../../include/clang/Basic/LangStandard.h"
#include "../../../../../../include/clang/Basic/OpenMPKinds.h"
#include "../../../../../../include/clang/Basic/OperatorKinds.h"
#include "../../../../../../include/clang/Basic/PragmaKinds.h"
#include "../../../../../../include/clang/Basic/Sanitizers.h"
#include "../../../../../../include/clang/Basic/Sarif.h"
#include "../../../../../../include/clang/Basic/SyncScope.h"
#include "../../../../../../include/clang/Basic/TargetCXXABI.h"
#include "../../../../../../include/clang/Basic/TokenKinds.h"
#include "../../../../../../include/clang/Basic/Version.h"
#include "../../../../../../include/clang/Lex/DependencyDirectivesScanner.h"
#include "../../../../../../include/clang/Lex/MacroBase.h"
#include "../../../../../../include/clang/Lex/MultipleIncludeOpt.h"
#include "../../../../../../include/clang/Lex/PreprocessorLexer.h"
#include "../../../../../../include/clang/Lex/Token.h"
#include "../../../../../../include/clang/ScalableStaticAnalysis/Core/Model/BuildNamespace.h"
#include "../../../../../../include/clang/ScalableStaticAnalysis/Core/Model/EntityIdTable.h"
#include "../../../../../../include/clang/ScalableStaticAnalysis/Core/Support/ErrorBuilder.h"
#include "../../../../../../include/clang/ScalableStaticAnalysis/Core/WholeProgramAnalysis/AnalysisResult.h"
#include "../../../../../../include/clang/ScalableStaticAnalysis/Core/WholeProgramAnalysis/AnalysisTraits.h"
#include "../../../../../../include/clang/ScalableStaticAnalysis/SourceTransformation/SourceEditEmitter.h"
#include "../../../../../../include/clang/ScalableStaticAnalysis/SourceTransformation/TransformationReportEmitter.h"
#include "../../../../../../include/clang/Tooling/Core/Replacement.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Sarif.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"
#include "clang/ScalableStaticAnalysis/Core/Model/EntityIdTable.h"
#include "clang/ScalableStaticAnalysis/Core/WholeProgramAnalysis/WPASuite.h"
#include "clang/ScalableStaticAnalysis/SourceTransformation/Transformation.h"
#include "clang/ScalableStaticAnalysis/SourceTransformation/TransformationRegistry.h"
#include "clang/Tooling/Core/Replacement.h"

using namespace clang;
using namespace clang::ssaf;

namespace {

class TestTransformation final : public Transformation {
public:
  using Transformation::Transformation;

  void HandleTranslationUnit(ASTContext &Ctx) override {
    bool SuiteIsNonEmpty = Suite.getIdTable().count() > 0;
    Visitor V{*this, Ctx, SuiteIsNonEmpty};
    V.TraverseDecl(Ctx.getTranslationUnitDecl());
  }

private:
  class Visitor : public RecursiveASTVisitor<Visitor> {
  public:
    Visitor(TestTransformation &T, ASTContext &Ctx, bool SuiteIsNonEmpty)
        : T(T), Ctx(Ctx), Level(SuiteIsNonEmpty
                                    ? clang::SarifResultLevel::Warning
                                    : clang::SarifResultLevel::Note) {}

    bool VisitFunctionDecl(FunctionDecl *FD) {
      if (!FD->hasBody())
        return true;
      SourceManager &SM = Ctx.getSourceManager();
      if (!SM.isInMainFile(FD->getLocation()))
        return true;

      Stmt *Body = FD->getBody();
      SourceLocation BodyStart = Body->getBeginLoc();
      if (BodyStart.isInvalid())
        return true;

      llvm::SmallString<64> FilePath(SM.getFilename(BodyStart));
      unsigned Offset = SM.getFileOffset(BodyStart);
      T.Edits.addReplacement(
          clang::tooling::Replacement(FilePath, Offset, /*Length=*/0, "/*T*/"));

      CharSourceRange Range = Lexer::getAsCharRange(
          CharSourceRange::getTokenRange(FD->getNameInfo().getSourceRange()),
          SM, Ctx.getLangOpts());
      std::string Message = "visited " + FD->getNameAsString();
      T.Report.addResult("test-touches-function", Level, Range, Message);
      return true;
    }

  private:
    TestTransformation &T;
    ASTContext &Ctx;
    clang::SarifResultLevel Level;
  };
};

} // namespace

namespace clang::ssaf {
// NOLINTNEXTLINE(misc-use-internal-linkage)
volatile int SSAFTestTransformationAnchorSource = 0;
} // namespace clang::ssaf

// This global causes issue in stage2 with ASan-instrumented clang so
// adding the no-ASan attribute.
static TransformationRegistry::Add<TestTransformation>
    __attribute__((no_sanitize("address")))
    RegisterTestTransformation("test-transformation",
                               "Test transformation for the SSAF "
                               "source-edit-generation lit suite");

