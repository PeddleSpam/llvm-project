//===-- ClangPersistentVariables.cpp --------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../../../../../clang/include/clang/APINotes/APINotesManager.h"
#include "../../../../../clang/include/clang/AST/ASTContextAllocate.h"
#include "../../../../../clang/include/clang/AST/ASTFwd.h"
#include "../../../../../clang/include/clang/AST/ASTImportError.h"
#include "../../../../../clang/include/clang/AST/Attr.h"
#include "../../../../../clang/include/clang/AST/CanonicalType.h"
#include "../../../../../clang/include/clang/AST/CommentCommandTraits.h"
#include "../../../../../clang/include/clang/AST/DeclBase.h"
#include "../../../../../clang/include/clang/AST/DeclCXX.h"
#include "../../../../../clang/include/clang/AST/DeclFriend.h"
#include "../../../../../clang/include/clang/AST/DeclID.h"
#include "../../../../../clang/include/clang/AST/DeclObjC.h"
#include "../../../../../clang/include/clang/AST/Expr.h"
#include "../../../../../clang/include/clang/AST/ExprCXX.h"
#include "../../../../../clang/include/clang/AST/ExprConcepts.h"
#include "../../../../../clang/include/clang/AST/NestedNameSpecifierBase.h"
#include "../../../../../clang/include/clang/AST/OperationKinds.h"
#include "../../../../../clang/include/clang/AST/PrettyPrinter.h"
#include "../../../../../clang/include/clang/AST/RawCommentList.h"
#include "../../../../../clang/include/clang/AST/SYCLKernelInfo.h"
#include "../../../../../clang/include/clang/AST/Stmt.h"
#include "../../../../../clang/include/clang/AST/TypeBase.h"
#include "../../../../../clang/include/clang/AST/TypeLoc.h"
#include "../../../../../clang/include/clang/AST/TypeOrdering.h"
#include "../../../../../clang/include/clang/Analysis/Analyses/LifetimeSafety/LifetimeAnnotations.h"
#include "../../../../../clang/include/clang/Analysis/Analyses/LifetimeSafety/LifetimeStats.h"
#include "../../../../../clang/include/clang/Basic/AttrKinds.h"
#include "../../../../../clang/include/clang/Basic/AttrSubjectMatchRules.h"
#include "../../../../../clang/include/clang/Basic/AttributeCommonInfo.h"
#include "../../../../../clang/include/clang/Basic/Builtins.h"
#include "../../../../../clang/include/clang/Basic/CFProtectionOptions.h"
#include "../../../../../clang/include/clang/Basic/CodeGenOptions.h"
#include "../../../../../clang/include/clang/Basic/CustomizableOptional.h"
#include "../../../../../clang/include/clang/Basic/Diagnostic.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticCategories.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticIDs.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticOptions.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticSema.h"
#include "../../../../../clang/include/clang/Basic/FileManager.h"
#include "../../../../../clang/include/clang/Basic/LangOptions.h"
#include "../../../../../clang/include/clang/Basic/LangStandard.h"
#include "../../../../../clang/include/clang/Basic/OffloadArch.h"
#include "../../../../../clang/include/clang/Basic/OpenMPKinds.h"
#include "../../../../../clang/include/clang/Basic/OperatorKinds.h"
#include "../../../../../clang/include/clang/Basic/ParsedAttrInfo.h"
#include "../../../../../clang/include/clang/Basic/Sanitizers.h"
#include "../../../../../clang/include/clang/Basic/TargetCXXABI.h"
#include "../../../../../clang/include/clang/Basic/TargetInfo.h"
#include "../../../../../clang/include/clang/Basic/TargetOptions.h"
#include "../../../../../clang/include/clang/Basic/TokenKinds.h"
#include "../../../../../clang/include/clang/Lex/MacroBase.h"
#include "../../../../../clang/include/clang/Sema/DeclSpec.h"
#include "../../../../../clang/include/clang/Sema/ParsedAttr.h"
#include "../../../../../clang/include/clang/Sema/ScopeInfo.h"
#include "../../../../../clang/include/clang/Sema/Sema.h"
#include "../../../../../llvm/include/llvm/ADT/APFixedPoint.h"
#include "../../../../../llvm/include/llvm/Frontend/HLSL/HLSLRootSignature.h"
#include "../../../../../llvm/include/llvm/Support/TypeSize.h"
#include "../../../../../llvm/include/llvm/TargetParser/AtomicScope.h"
#include "ClangASTImporter.h"

#include "Plugins/TypeSystem/Clang/TypeSystemClang.h"


#include <optional>
#include <memory>

using namespace lldb;
using namespace lldb_private;

char ClangPersistentVariables::ID;

ClangPersistentVariables::ClangPersistentVariables(
    std::shared_ptr<Target> target_sp)
    : m_target_sp(target_sp) {}

ExpressionVariableSP ClangPersistentVariables::CreatePersistentVariable(
    const lldb::ValueObjectSP &valobj_sp) {
  return AddNewlyConstructedVariable(new ClangExpressionVariable(valobj_sp));
}

ExpressionVariableSP ClangPersistentVariables::CreatePersistentVariable(
    ExecutionContextScope *exe_scope, ConstString name,
    const CompilerType &compiler_type, lldb::ByteOrder byte_order,
    uint32_t addr_byte_size) {
  return AddNewlyConstructedVariable(new ClangExpressionVariable(
      exe_scope, name, compiler_type, byte_order, addr_byte_size));
}

void ClangPersistentVariables::RemovePersistentVariable(
    lldb::ExpressionVariableSP variable) {
  RemoveVariable(variable);

  // Check if the removed variable was the last one that was created. If yes,
  // reuse the variable id for the next variable.

  // Nothing to do if we have not assigned a variable id so far.
  if (m_next_persistent_variable_id == 0)
    return;

  llvm::StringRef name = variable->GetName().GetStringRef();
  // Remove the prefix from the variable that only the indes is left.
  if (!name.consume_front(GetPersistentVariablePrefix(false)))
    return;

  // Check if the variable contained a variable id.
  uint32_t variable_id;
  if (name.getAsInteger(10, variable_id))
    return;
  // If it's the most recent variable id that was assigned, make sure that this
  // variable id will be used for the next persistent variable.
  if (variable_id == m_next_persistent_variable_id - 1)
    m_next_persistent_variable_id--;
}

std::optional<CompilerType>
ClangPersistentVariables::GetCompilerTypeFromPersistentDecl(
    ConstString type_name) {
  PersistentDecl p = m_persistent_decls.lookup(type_name.GetCString());

  if (p.m_decl == nullptr)
    return std::nullopt;

  auto ctx = std::static_pointer_cast<TypeSystemClang>(p.m_context.lock());
  if (clang::TypeDecl *tdecl = llvm::dyn_cast<clang::TypeDecl>(p.m_decl)) {
    opaque_compiler_type_t t =
        static_cast<opaque_compiler_type_t>(const_cast<clang::Type *>(
            ctx->getASTContext().getTypeDeclType(tdecl).getTypePtr()));
    return CompilerType(p.m_context, t);
  }
  return std::nullopt;
}

void ClangPersistentVariables::RegisterPersistentDecl(
    ConstString name, clang::NamedDecl *decl,
    std::shared_ptr<TypeSystemClang> ctx) {
  PersistentDecl p = {decl, ctx};
  m_persistent_decls.insert(std::make_pair(name.GetCString(), p));

  if (clang::EnumDecl *enum_decl = llvm::dyn_cast<clang::EnumDecl>(decl)) {
    for (clang::EnumConstantDecl *enumerator_decl : enum_decl->enumerators()) {
      p = {enumerator_decl, ctx};
      m_persistent_decls.insert(std::make_pair(
          ConstString(enumerator_decl->getNameAsString()).GetCString(), p));
    }
  }
}

clang::NamedDecl *
ClangPersistentVariables::GetPersistentDecl(ConstString name) {
  return m_persistent_decls.lookup(name.GetCString()).m_decl;
}

std::shared_ptr<ClangASTImporter>
ClangPersistentVariables::GetClangASTImporter() {
  if (!m_ast_importer_sp) {
    m_ast_importer_sp = std::make_shared<ClangASTImporter>();
  }
  return m_ast_importer_sp;
}

std::shared_ptr<ClangModulesDeclVendor>
ClangPersistentVariables::GetClangModulesDeclVendor() {
  if (!m_modules_decl_vendor_sp) {
    m_modules_decl_vendor_sp.reset(
        ClangModulesDeclVendor::Create(*m_target_sp));
  }
  return m_modules_decl_vendor_sp;
}

ConstString
ClangPersistentVariables::GetNextPersistentVariableName(bool is_error) {
  llvm::SmallString<64> name;
  {
    llvm::raw_svector_ostream os(name);
    os << GetPersistentVariablePrefix(is_error)
       << m_next_persistent_variable_id++;
  }
  return ConstString(name);
}
