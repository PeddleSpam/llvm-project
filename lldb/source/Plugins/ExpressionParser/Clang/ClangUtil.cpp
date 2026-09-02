//===-- ClangUtil.cpp -----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// A collection of helper methods and data structures for manipulating clang
// types and decls.
//===----------------------------------------------------------------------===//

#include "Plugins/ExpressionParser/Clang/ClangUtil.h"
#include "../../../../../clang/include/clang/AST/APNumericStorage.h"
#include "../../../../../clang/include/clang/AST/APValue.h"
#include "../../../../../clang/include/clang/AST/ASTContext.h"
#include "../../../../../clang/include/clang/AST/ASTContextAllocate.h"
#include "../../../../../clang/include/clang/AST/ASTDumperUtils.h"
#include "../../../../../clang/include/clang/AST/ASTFwd.h"
#include "../../../../../clang/include/clang/AST/ASTVector.h"
#include "../../../../../clang/include/clang/AST/Attr.h"
#include "../../../../../clang/include/clang/AST/CharUnits.h"
#include "../../../../../clang/include/clang/AST/CommentCommandTraits.h"
#include "../../../../../clang/include/clang/AST/ComputeDependence.h"
#include "../../../../../clang/include/clang/AST/DeclAccessPair.h"
#include "../../../../../clang/include/clang/AST/DeclCXX.h"
#include "../../../../../clang/include/clang/AST/DeclGroup.h"
#include "../../../../../clang/include/clang/AST/DeclID.h"
#include "../../../../../clang/include/clang/AST/OperationKinds.h"
#include "../../../../../clang/include/clang/AST/SelectorLocationsKind.h"
#include "../../../../../clang/include/clang/AST/StmtIterator.h"
#include "../../../../../clang/include/clang/AST/TypeBase.h"
#include "../../../../../clang/include/clang/AST/TypeLoc.h"
#include "../../../../../clang/include/clang/Basic/AttributeCommonInfo.h"
#include "../../../../../clang/include/clang/Basic/BuiltinTraits.h"
#include "../../../../../clang/include/clang/Basic/Builtins.h"
#include "../../../../../clang/include/clang/Basic/CFProtectionOptions.h"
#include "../../../../../clang/include/clang/Basic/CapturedStmt.h"
#include "../../../../../clang/include/clang/Basic/CharInfo.h"
#include "../../../../../clang/include/clang/Basic/CodeGenOptions.h"
#include "../../../../../clang/include/clang/Basic/CustomizableOptional.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticCategories.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticIDs.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticOptions.h"
#include "../../../../../clang/include/clang/Basic/DirectoryEntry.h"
#include "../../../../../clang/include/clang/Basic/FileEntry.h"
#include "../../../../../clang/include/clang/Basic/IdentifierTable.h"
#include "../../../../../clang/include/clang/Basic/LangOptions.h"
#include "../../../../../clang/include/clang/Basic/LangStandard.h"
#include "../../../../../clang/include/clang/Basic/OpenCLOptions.h"
#include "../../../../../clang/include/clang/Basic/OpenMPKinds.h"
#include "../../../../../clang/include/clang/Basic/OperatorKinds.h"
#include "../../../../../clang/include/clang/Basic/OptionalUnsigned.h"
#include "../../../../../clang/include/clang/Basic/PragmaKinds.h"
#include "../../../../../clang/include/clang/Basic/Sanitizers.h"
#include "../../../../../clang/include/clang/Basic/SyncScope.h"
#include "../../../../../clang/include/clang/Basic/TargetCXXABI.h"
#include "../../../../../clang/include/clang/Basic/TargetInfo.h"
#include "../../../../../clang/include/clang/Basic/TokenKinds.h"
#include "../../../../../llvm/include/llvm/ADT/APFixedPoint.h"
#include "../../../../../llvm/include/llvm/ADT/FunctionExtras.h"
#include "../../../../../llvm/include/llvm/Frontend/HLSL/HLSLRootSignature.h"
#include "../../../../../llvm/include/llvm/Support/AtomicOrdering.h"
#include "../../../../../llvm/include/llvm/Support/ConvertUTF.h"
#include "../../../../../llvm/include/llvm/Support/PrettyStackTrace.h"
#include "../../../../../llvm/include/llvm/TargetParser/AtomicScope.h"
#include "Plugins/TypeSystem/Clang/TypeSystemClang.h"

using namespace clang;
using namespace lldb_private;

bool ClangUtil::IsClangType(const CompilerType &ct) {
  // Invalid types are never Clang types.
  if (!ct)
    return false;

  if (!ct.GetTypeSystem<TypeSystemClang>())
    return false;

  if (!ct.GetOpaqueQualType())
    return false;

  return true;
}

clang::Decl *ClangUtil::GetDecl(const CompilerDecl &decl) {
  assert(llvm::isa<TypeSystemClang>(decl.GetTypeSystem()));
  return static_cast<clang::Decl *>(decl.GetOpaqueDecl());
}

QualType ClangUtil::GetQualType(const CompilerType &ct) {
  // Make sure we have a clang type before making a clang::QualType
  if (!IsClangType(ct))
    return QualType();

  return QualType::getFromOpaquePtr(ct.GetOpaqueQualType());
}

QualType ClangUtil::GetCanonicalQualType(const CompilerType &ct) {
  if (!IsClangType(ct))
    return QualType();

  return GetQualType(ct).getCanonicalType();
}

CompilerType ClangUtil::RemoveFastQualifiers(const CompilerType &ct) {
  if (!IsClangType(ct))
    return ct;

  QualType qual_type(GetQualType(ct));
  qual_type.removeLocalFastQualifiers();
  return CompilerType(ct.GetTypeSystem(), qual_type.getAsOpaquePtr());
}

clang::TagDecl *ClangUtil::GetAsTagDecl(const CompilerType &type) {
  clang::QualType qual_type = ClangUtil::GetCanonicalQualType(type);
  if (qual_type.isNull())
    return nullptr;

  return qual_type->getAsTagDecl();
}

std::string ClangUtil::DumpDecl(const clang::Decl *d) {
  if (!d)
    return "nullptr";

  std::string result;
  llvm::raw_string_ostream stream(result);
  bool deserialize = false;
  d->dump(stream, deserialize);

  return result;
}

std::string ClangUtil::ToString(const clang::Type *t) {
  return clang::QualType(t, 0).getAsString();
}

std::string ClangUtil::ToString(const CompilerType &c) {
  return ClangUtil::GetQualType(c).getAsString();
}
