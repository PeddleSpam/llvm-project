//===-- ASTStructExtractor.cpp --------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ASTStructExtractor.h"

#include "../../../../../clang/include/clang-c/BuildSystem.h"
#include "../../../../../clang/include/clang-c/CXDiagnostic.h"
#include "../../../../../clang/include/clang-c/CXFile.h"
#include "../../../../../clang/include/clang-c/CXSourceLocation.h"
#include "../../../../../clang/include/clang-c/Index.h"
#include "../../../../../clang/include/clang/AST/ASTConsumer.h"
#include "../../../../../clang/include/clang/AST/ASTContextAllocate.h"
#include "../../../../../clang/include/clang/AST/Attr.h"
#include "../../../../../clang/include/clang/AST/CanonicalType.h"
#include "../../../../../clang/include/clang/AST/CommentCommandTraits.h"
#include "../../../../../clang/include/clang/AST/DeclBase.h"
#include "../../../../../clang/include/clang/AST/DeclCXX.h"
#include "../../../../../clang/include/clang/AST/DeclFriend.h"
#include "../../../../../clang/include/clang/AST/DeclObjC.h"
#include "../../../../../clang/include/clang/AST/Expr.h"
#include "../../../../../clang/include/clang/AST/ExprCXX.h"
#include "../../../../../clang/include/clang/AST/ExprConcepts.h"
#include "../../../../../clang/include/clang/AST/ExprOpenMP.h"
#include "../../../../../clang/include/clang/AST/ExternalASTSource.h"
#include "../../../../../clang/include/clang/AST/NSAPI.h"
#include "../../../../../clang/include/clang/AST/NestedNameSpecifierBase.h"
#include "../../../../../clang/include/clang/AST/OpenMPClause.h"
#include "../../../../../clang/include/clang/AST/OperationKinds.h"
#include "../../../../../clang/include/clang/AST/PrettyPrinter.h"
#include "../../../../../clang/include/clang/AST/RawCommentList.h"
#include "../../../../../clang/include/clang/AST/SYCLKernelInfo.h"
#include "../../../../../clang/include/clang/AST/Stmt.h"
#include "../../../../../clang/include/clang/AST/StmtOpenMP.h"
#include "../../../../../clang/include/clang/AST/TypeBase.h"
#include "../../../../../clang/include/clang/AST/TypeLoc.h"
#include "../../../../../clang/include/clang/AST/TypeOrdering.h"
#include "../../../../../clang/include/clang/Analysis/Analyses/LifetimeSafety/LifetimeAnnotations.h"
#include "../../../../../clang/include/clang/Analysis/Analyses/LifetimeSafety/LifetimeStats.h"
#include "../../../../../clang/include/clang/Basic/AttrKinds.h"
#include "../../../../../clang/include/clang/Basic/AttrSubjectMatchRules.h"
#include "../../../../../clang/include/clang/Basic/AttributeCommonInfo.h"
#include "../../../../../clang/include/clang/Basic/BitmaskEnum.h"
#include "../../../../../clang/include/clang/Basic/BuiltinTraits.h"
#include "../../../../../clang/include/clang/Basic/Builtins.h"
#include "../../../../../clang/include/clang/Basic/CFProtectionOptions.h"
#include "../../../../../clang/include/clang/Basic/CustomizableOptional.h"
#include "../../../../../clang/include/clang/Basic/Diagnostic.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticCategories.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticIDs.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticOptions.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticSema.h"
#include "../../../../../clang/include/clang/Basic/FileManager.h"
#include "../../../../../clang/include/clang/Basic/LangOptions.h"
#include "../../../../../clang/include/clang/Basic/LangStandard.h"
#include "../../../../../clang/include/clang/Basic/Module.h"
#include "../../../../../clang/include/clang/Basic/OffloadArch.h"
#include "../../../../../clang/include/clang/Basic/OpenMPKinds.h"
#include "../../../../../clang/include/clang/Basic/OperatorKinds.h"
#include "../../../../../clang/include/clang/Basic/ParsedAttrInfo.h"
#include "../../../../../clang/include/clang/Basic/Sanitizers.h"
#include "../../../../../clang/include/clang/Basic/SourceManager.h"
#include "../../../../../clang/include/clang/Basic/TargetCXXABI.h"
#include "../../../../../clang/include/clang/Basic/TokenKinds.h"
#include "../../../../../clang/include/clang/Lex/DependencyDirectivesScanner.h"
#include "../../../../../clang/include/clang/Lex/DirectoryLookup.h"
#include "../../../../../clang/include/clang/Lex/ExternalPreprocessorSource.h"
#include "../../../../../clang/include/clang/Lex/HeaderMap.h"
#include "../../../../../clang/include/clang/Lex/HeaderMapTypes.h"
#include "../../../../../clang/include/clang/Lex/HeaderSearch.h"
#include "../../../../../clang/include/clang/Lex/Lexer.h"
#include "../../../../../clang/include/clang/Lex/MacroBase.h"
#include "../../../../../clang/include/clang/Lex/MacroInfo.h"
#include "../../../../../clang/include/clang/Lex/ModuleLoader.h"
#include "../../../../../clang/include/clang/Lex/ModuleMap.h"
#include "../../../../../clang/include/clang/Lex/MultipleIncludeOpt.h"
#include "../../../../../clang/include/clang/Lex/PPCallbacks.h"
#include "../../../../../clang/include/clang/Lex/PPDirectiveParameter.h"
#include "../../../../../clang/include/clang/Lex/PPEmbedParameters.h"
#include "../../../../../clang/include/clang/Lex/Pragma.h"
#include "../../../../../clang/include/clang/Lex/PreprocessorLexer.h"
#include "../../../../../clang/include/clang/Lex/TextEncoding.h"
#include "../../../../../clang/include/clang/Lex/TokenLexer.h"
#include "../../../../../clang/include/clang/Parse/Parser.h"
#include "../../../../../clang/include/clang/Sema/CodeCompleteConsumer.h"
#include "../../../../../clang/include/clang/Sema/CodeCompleteOptions.h"
#include "../../../../../clang/include/clang/Sema/DeclSpec.h"
#include "../../../../../clang/include/clang/Sema/Designator.h"
#include "../../../../../clang/include/clang/Sema/HeuristicResolver.h"
#include "../../../../../clang/include/clang/Sema/ObjCMethodList.h"
#include "../../../../../clang/include/clang/Sema/ParsedAttr.h"
#include "../../../../../clang/include/clang/Sema/ScopeInfo.h"
#include "../../../../../clang/include/clang/Sema/Sema.h"
#include "../../../../../llvm/include/llvm/ADT/APFixedPoint.h"
#include "../../../../../llvm/include/llvm/ADT/PagedVector.h"
#include "../../../../../llvm/include/llvm/Frontend/HLSL/HLSLRootSignature.h"
#include "../../../../../llvm/include/llvm/Support/TypeSize.h"
#include "../../../../../llvm/include/llvm/Target/TargetOptions.h"
#include "../../../../../llvm/include/llvm/TargetParser/AtomicScope.h"
#include "clang/AST/RecordLayout.h"
#include "clang/Parse/Parser.h"
#include <cstdlib>

using namespace llvm;
using namespace clang;
using namespace lldb_private;

ASTStructExtractor::ASTStructExtractor(ASTConsumer *passthrough,
                                       const char *struct_name,
                                       ClangFunctionCaller &function)
    : m_ast_context(nullptr), m_passthrough(passthrough),
      m_passthrough_sema(nullptr), m_sema(nullptr), m_function(function),
      m_struct_name(struct_name) {
  if (!m_passthrough)
    return;

  m_passthrough_sema = dyn_cast<SemaConsumer>(passthrough);
}

ASTStructExtractor::~ASTStructExtractor() = default;

void ASTStructExtractor::Initialize(ASTContext &Context) {
  m_ast_context = &Context;

  if (m_passthrough)
    m_passthrough->Initialize(Context);
}

void ASTStructExtractor::ExtractFromFunctionDecl(FunctionDecl *F) {
  if (!F->hasBody())
    return;

  Stmt *body_stmt = F->getBody();
  CompoundStmt *body_compound_stmt = dyn_cast<CompoundStmt>(body_stmt);

  if (!body_compound_stmt)
    return; // do we have to handle this?

  RecordDecl *struct_decl = nullptr;

  StringRef desired_name(m_struct_name);

  for (CompoundStmt::const_body_iterator bi = body_compound_stmt->body_begin(),
                                         be = body_compound_stmt->body_end();
       bi != be; ++bi) {
    Stmt *curr_stmt = *bi;
    DeclStmt *curr_decl_stmt = dyn_cast<DeclStmt>(curr_stmt);
    if (!curr_decl_stmt)
      continue;
    DeclGroupRef decl_group = curr_decl_stmt->getDeclGroup();
    for (Decl *candidate_decl : decl_group) {
      RecordDecl *candidate_record_decl = dyn_cast<RecordDecl>(candidate_decl);
      if (!candidate_record_decl)
        continue;
      if (candidate_record_decl->getName() == desired_name) {
        struct_decl = candidate_record_decl;
        break;
      }
    }
    if (struct_decl)
      break;
  }

  if (!struct_decl)
    return;

  const ASTRecordLayout *struct_layout(
      &m_ast_context->getASTRecordLayout(struct_decl));

  if (!struct_layout)
    return;

  m_function.m_struct_size =
      struct_layout->getSize()
          .getQuantity(); // TODO Store m_struct_size as CharUnits
  m_function.m_return_offset =
      struct_layout->getFieldOffset(struct_layout->getFieldCount() - 1) / 8;
  m_function.m_return_size =
      struct_layout->getDataSize().getQuantity() - m_function.m_return_offset;

  for (unsigned field_index = 0, num_fields = struct_layout->getFieldCount();
       field_index < num_fields; ++field_index) {
    m_function.m_member_offsets.push_back(
        struct_layout->getFieldOffset(field_index) / 8);
  }

  m_function.m_struct_valid = true;
}

void ASTStructExtractor::ExtractFromTopLevelDecl(Decl *D) {
  LinkageSpecDecl *linkage_spec_decl = dyn_cast<LinkageSpecDecl>(D);

  if (linkage_spec_decl) {
    RecordDecl::decl_iterator decl_iterator;

    for (decl_iterator = linkage_spec_decl->decls_begin();
         decl_iterator != linkage_spec_decl->decls_end(); ++decl_iterator) {
      ExtractFromTopLevelDecl(*decl_iterator);
    }
  }

  FunctionDecl *function_decl = dyn_cast<FunctionDecl>(D);

  if (m_ast_context && function_decl &&
      m_function.m_wrapper_function_name == function_decl->getNameAsString()) {
    ExtractFromFunctionDecl(function_decl);
  }
}

bool ASTStructExtractor::HandleTopLevelDecl(DeclGroupRef D) {
  DeclGroupRef::iterator decl_iterator;

  for (decl_iterator = D.begin(); decl_iterator != D.end(); ++decl_iterator) {
    Decl *decl = *decl_iterator;

    ExtractFromTopLevelDecl(decl);
  }

  if (m_passthrough)
    return m_passthrough->HandleTopLevelDecl(D);
  return true;
}

void ASTStructExtractor::HandleTranslationUnit(ASTContext &Ctx) {
  if (m_passthrough)
    m_passthrough->HandleTranslationUnit(Ctx);
}

void ASTStructExtractor::HandleTagDeclDefinition(TagDecl *D) {
  if (m_passthrough)
    m_passthrough->HandleTagDeclDefinition(D);
}

void ASTStructExtractor::CompleteTentativeDefinition(VarDecl *D) {
  if (m_passthrough)
    m_passthrough->CompleteTentativeDefinition(D);
}

void ASTStructExtractor::HandleVTable(CXXRecordDecl *RD) {
  if (m_passthrough)
    m_passthrough->HandleVTable(RD);
}

void ASTStructExtractor::PrintStats() {
  if (m_passthrough)
    m_passthrough->PrintStats();
}

void ASTStructExtractor::InitializeSema(Sema &S) {
  m_sema = &S;

  if (m_passthrough_sema)
    m_passthrough_sema->InitializeSema(S);
}

void ASTStructExtractor::ForgetSema() {
  m_sema = nullptr;

  if (m_passthrough_sema)
    m_passthrough_sema->ForgetSema();
}
