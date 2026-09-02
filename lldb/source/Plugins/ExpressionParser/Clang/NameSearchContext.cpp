//===-- NameSearchContext.cpp ---------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "NameSearchContext.h"
#include "../../../../../clang/include/clang/AST/ASTContextAllocate.h"
#include "../../../../../clang/include/clang/AST/ASTFwd.h"
#include "../../../../../clang/include/clang/AST/ASTImportError.h"
#include "../../../../../clang/include/clang/AST/Attr.h"
#include "../../../../../clang/include/clang/AST/CanonicalType.h"
#include "../../../../../clang/include/clang/AST/CommentCommandTraits.h"
#include "../../../../../clang/include/clang/AST/ComparisonCategories.h"
#include "../../../../../clang/include/clang/AST/DeclBase.h"
#include "../../../../../clang/include/clang/AST/DeclCXX.h"
#include "../../../../../clang/include/clang/AST/DeclFriend.h"
#include "../../../../../clang/include/clang/AST/DeclObjC.h"
#include "../../../../../clang/include/clang/AST/Expr.h"
#include "../../../../../clang/include/clang/AST/OperationKinds.h"
#include "../../../../../clang/include/clang/AST/PrettyPrinter.h"
#include "../../../../../clang/include/clang/AST/RawCommentList.h"
#include "../../../../../clang/include/clang/AST/SYCLKernelInfo.h"
#include "../../../../../clang/include/clang/AST/Stmt.h"
#include "../../../../../clang/include/clang/AST/TypeBase.h"
#include "../../../../../clang/include/clang/AST/TypeLoc.h"
#include "../../../../../clang/include/clang/AST/TypeOrdering.h"
#include "../../../../../clang/include/clang/Analysis/Analyses/LifetimeSafety/LifetimeStats.h"
#include "../../../../../clang/include/clang/Basic/AttrKinds.h"
#include "../../../../../clang/include/clang/Basic/AttrSubjectMatchRules.h"
#include "../../../../../clang/include/clang/Basic/AttributeCommonInfo.h"
#include "../../../../../clang/include/clang/Basic/Builtins.h"
#include "../../../../../clang/include/clang/Basic/CFProtectionOptions.h"
#include "../../../../../clang/include/clang/Basic/CodeGenOptions.h"
#include "../../../../../clang/include/clang/Basic/CustomizableOptional.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticCategories.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticIDs.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticOptions.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticSema.h"
#include "../../../../../clang/include/clang/Basic/LangOptions.h"
#include "../../../../../clang/include/clang/Basic/LangStandard.h"
#include "../../../../../clang/include/clang/Basic/Module.h"
#include "../../../../../clang/include/clang/Basic/OffloadArch.h"
#include "../../../../../clang/include/clang/Basic/OpenMPKinds.h"
#include "../../../../../clang/include/clang/Basic/OperatorKinds.h"
#include "../../../../../clang/include/clang/Basic/OptionalUnsigned.h"
#include "../../../../../clang/include/clang/Basic/Sanitizers.h"
#include "../../../../../clang/include/clang/Basic/TargetCXXABI.h"
#include "../../../../../clang/include/clang/Basic/TargetInfo.h"
#include "../../../../../clang/include/clang/Basic/TokenKinds.h"
#include "../../../../../clang/include/clang/Lex/MacroBase.h"
#include "../../../../../clang/include/clang/Sema/DeclSpec.h"
#include "../../../../../clang/include/clang/Sema/ParsedAttr.h"
#include "../../../../../clang/include/clang/Sema/ScopeInfo.h"
#include "../../../../../clang/include/clang/Sema/Sema.h"
#include "../../../../../llvm/include/llvm/ADT/APFixedPoint.h"
#include "../../../../../llvm/include/llvm/ADT/FunctionExtras.h"
#include "../../../../../llvm/include/llvm/ADT/TypeSwitch.h"
#include "../../../../../llvm/include/llvm/Frontend/HLSL/HLSLRootSignature.h"
#include "../../../../../llvm/include/llvm/Support/ConvertUTF.h"
#include "../../../../../llvm/include/llvm/Support/Registry.h"
#include "../../../../../llvm/include/llvm/Support/TypeSize.h"
#include "../../../../../llvm/include/llvm/TargetParser/AtomicScope.h"
#include "ClangUtil.h"

using namespace clang;
using namespace lldb_private;

clang::NamedDecl *NameSearchContext::AddVarDecl(const CompilerType &type) {
  assert(type && "Type for variable must be valid!");

  if (!type.IsValid())
    return nullptr;

  auto lldb_ast = type.GetTypeSystem<TypeSystemClang>();
  if (!lldb_ast)
    return nullptr;

  IdentifierInfo *ii = m_decl_name.getAsIdentifierInfo();

  clang::ASTContext &ast = lldb_ast->getASTContext();

  clang::NamedDecl *Decl = VarDecl::Create(
      ast, const_cast<DeclContext *>(m_decl_context), SourceLocation(),
      SourceLocation(), ii, ClangUtil::GetQualType(type), nullptr, SC_Static);
  m_decls.push_back(Decl);

  return Decl;
}

clang::NamedDecl *NameSearchContext::AddFunDecl(const CompilerType &type,
                                                bool extern_c) {
  assert(type && "Type for variable must be valid!");

  if (!type.IsValid())
    return nullptr;

  if (m_function_types.count(type))
    return nullptr;

  auto lldb_ast = type.GetTypeSystem<TypeSystemClang>();
  if (!lldb_ast)
    return nullptr;

  m_function_types.insert(type);

  QualType qual_type(ClangUtil::GetQualType(type));

  clang::ASTContext &ast = lldb_ast->getASTContext();

  const bool isInlineSpecified = false;
  const bool hasWrittenPrototype = true;
  const bool isConstexprSpecified = false;

  clang::DeclContext *context = const_cast<DeclContext *>(m_decl_context);

  if (extern_c) {
    context = LinkageSpecDecl::Create(ast, context, SourceLocation(),
                                      SourceLocation(),
                                      clang::LinkageSpecLanguageIDs::C, false);
    // FIXME: The LinkageSpecDecl here should be added to m_decl_context.
  }

  // Pass the identifier info for functions the decl_name is needed for
  // operators
  clang::DeclarationName decl_name =
      m_decl_name.getNameKind() == DeclarationName::Identifier
          ? m_decl_name.getAsIdentifierInfo()
          : m_decl_name;

  clang::FunctionDecl *func_decl = FunctionDecl::Create(
      ast, context, SourceLocation(), SourceLocation(), decl_name, qual_type,
      nullptr, SC_Extern, /*UsesFPIntrin=*/false, isInlineSpecified, hasWrittenPrototype,
      isConstexprSpecified ? ConstexprSpecKind::Constexpr
                           : ConstexprSpecKind::Unspecified);

  // We have to do more than just synthesize the FunctionDecl.  We have to
  // synthesize ParmVarDecls for all of the FunctionDecl's arguments.  To do
  // this, we raid the function's FunctionProtoType for types.

  const FunctionProtoType *func_proto_type =
      qual_type.getTypePtr()->getAs<FunctionProtoType>();

  if (func_proto_type) {
    unsigned NumArgs = func_proto_type->getNumParams();
    unsigned ArgIndex;

    SmallVector<ParmVarDecl *, 5> parm_var_decls;

    for (ArgIndex = 0; ArgIndex < NumArgs; ++ArgIndex) {
      QualType arg_qual_type(func_proto_type->getParamType(ArgIndex));

      parm_var_decls.push_back(
          ParmVarDecl::Create(ast, const_cast<DeclContext *>(context),
                              SourceLocation(), SourceLocation(), nullptr,
                              arg_qual_type, nullptr, SC_Static, nullptr));
    }

    func_decl->setParams(ArrayRef<ParmVarDecl *>(parm_var_decls));
  } else {
    Log *log = GetLog(LLDBLog::Expressions);

    LLDB_LOG(log, "Function type wasn't a FunctionProtoType");
  }

  // If this is an operator (e.g. operator new or operator==), only insert the
  // declaration we inferred from the symbol if we can provide the correct
  // number of arguments. We shouldn't really inject random decl(s) for
  // functions that are analyzed semantically in a special way, otherwise we
  // will crash in clang.
  clang::OverloadedOperatorKind op_kind = clang::NUM_OVERLOADED_OPERATORS;
  if (func_proto_type &&
      TypeSystemClang::IsOperator(decl_name.getAsString().c_str(), op_kind)) {
    if (!TypeSystemClang::CheckOverloadedOperatorKindParameterCount(
            false, op_kind, func_proto_type->getNumParams()))
      return nullptr;
  }
  m_decls.push_back(func_decl);

  return func_decl;
}

clang::NamedDecl *NameSearchContext::AddGenericFunDecl() {
  FunctionProtoType::ExtProtoInfo proto_info;

  proto_info.Variadic = true;

  QualType generic_function_type(
      GetASTContext().getFunctionType(GetASTContext().UnknownAnyTy, // result
                                      ArrayRef<QualType>(), // argument types
                                      proto_info));

  return AddFunDecl(m_clang_ts.GetType(generic_function_type), true);
}

clang::NamedDecl *
NameSearchContext::AddTypeDecl(const CompilerType &clang_type) {
  if (ClangUtil::IsClangType(clang_type)) {
    QualType qual_type = ClangUtil::GetQualType(clang_type);

    if (const TypedefType *typedef_type =
            llvm::dyn_cast<TypedefType>(qual_type)) {
      TypedefNameDecl *typedef_name_decl = typedef_type->getDecl();

      m_decls.push_back(typedef_name_decl);

      return (NamedDecl *)typedef_name_decl;
    } else if (const TagType *tag_type = qual_type->getAs<TagType>()) {
      TagDecl *tag_decl = tag_type->getDecl()->getDefinitionOrSelf();

      m_decls.push_back(tag_decl);

      return tag_decl;
    } else if (const ObjCObjectType *objc_object_type =
                   qual_type->getAs<ObjCObjectType>()) {
      ObjCInterfaceDecl *interface_decl = objc_object_type->getInterface();

      m_decls.push_back((NamedDecl *)interface_decl);

      return (NamedDecl *)interface_decl;
    }
  }
  return nullptr;
}

void NameSearchContext::AddLookupResult(clang::DeclContextLookupResult result) {
  for (clang::NamedDecl *decl : result)
    m_decls.push_back(decl);
}

void NameSearchContext::AddNamedDecl(clang::NamedDecl *decl) {
  m_decls.push_back(decl);
}
