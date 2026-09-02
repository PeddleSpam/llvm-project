//===-- ASTUtils.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ASTUtils.h"
#include "../../../../../clang/include/clang/AST/APNumericStorage.h"
#include "../../../../../clang/include/clang/AST/APValue.h"
#include "../../../../../clang/include/clang/AST/ASTConsumer.h"
#include "../../../../../clang/include/clang/AST/ASTContext.h"
#include "../../../../../clang/include/clang/AST/ASTContextAllocate.h"
#include "../../../../../clang/include/clang/AST/ASTDumperUtils.h"
#include "../../../../../clang/include/clang/AST/ASTFwd.h"
#include "../../../../../clang/include/clang/AST/ASTVector.h"
#include "../../../../../clang/include/clang/AST/Attr.h"
#include "../../../../../clang/include/clang/AST/CommentCommandTraits.h"
#include "../../../../../clang/include/clang/AST/ComputeDependence.h"
#include "../../../../../clang/include/clang/AST/DeclAccessPair.h"
#include "../../../../../clang/include/clang/AST/DeclCXX.h"
#include "../../../../../clang/include/clang/AST/DeclFriend.h"
#include "../../../../../clang/include/clang/AST/DeclGroup.h"
#include "../../../../../clang/include/clang/AST/DeclID.h"
#include "../../../../../clang/include/clang/AST/DeclObjC.h"
#include "../../../../../clang/include/clang/AST/OperationKinds.h"
#include "../../../../../clang/include/clang/AST/RawCommentList.h"
#include "../../../../../clang/include/clang/AST/SelectorLocationsKind.h"
#include "../../../../../clang/include/clang/AST/StmtIterator.h"
#include "../../../../../clang/include/clang/AST/TypeBase.h"
#include "../../../../../clang/include/clang/AST/TypeLoc.h"
#include "../../../../../clang/include/clang/Analysis/Analyses/LifetimeSafety/LifetimeStats.h"
#include "../../../../../clang/include/clang/Basic/AttrSubjectMatchRules.h"
#include "../../../../../clang/include/clang/Basic/AttributeCommonInfo.h"
#include "../../../../../clang/include/clang/Basic/BitmaskEnum.h"
#include "../../../../../clang/include/clang/Basic/Builtins.h"
#include "../../../../../clang/include/clang/Basic/CFProtectionOptions.h"
#include "../../../../../clang/include/clang/Basic/CharInfo.h"
#include "../../../../../clang/include/clang/Basic/CustomizableOptional.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticCategories.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticIDs.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticOptions.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticSema.h"
#include "../../../../../clang/include/clang/Basic/DirectoryEntry.h"
#include "../../../../../clang/include/clang/Basic/IdentifierTable.h"
#include "../../../../../clang/include/clang/Basic/LangOptions.h"
#include "../../../../../clang/include/clang/Basic/LangStandard.h"
#include "../../../../../clang/include/clang/Basic/Module.h"
#include "../../../../../clang/include/clang/Basic/OffloadArch.h"
#include "../../../../../clang/include/clang/Basic/OpenMPKinds.h"
#include "../../../../../clang/include/clang/Basic/OperatorKinds.h"
#include "../../../../../clang/include/clang/Basic/OptionalUnsigned.h"
#include "../../../../../clang/include/clang/Basic/ParsedAttrInfo.h"
#include "../../../../../clang/include/clang/Basic/Sanitizers.h"
#include "../../../../../clang/include/clang/Basic/SyncScope.h"
#include "../../../../../clang/include/clang/Basic/TargetCXXABI.h"
#include "../../../../../clang/include/clang/Basic/TokenKinds.h"
#include "../../../../../clang/include/clang/Sema/DeclSpec.h"
#include "../../../../../clang/include/clang/Sema/ParsedAttr.h"
#include "../../../../../clang/include/clang/Sema/ScopeInfo.h"
#include "../../../../../clang/include/clang/Sema/Sema.h"
#include "../../../../../llvm/include/llvm/ADT/APFixedPoint.h"
#include "../../../../../llvm/include/llvm/ADT/APFloat.h"
#include "../../../../../llvm/include/llvm/ADT/FunctionExtras.h"
#include "../../../../../llvm/include/llvm/ADT/TypeSwitch.h"
#include "../../../../../llvm/include/llvm/Frontend/HLSL/HLSLRootSignature.h"
#include "../../../../../llvm/include/llvm/Support/AlignOf.h"
#include "../../../../../llvm/include/llvm/Support/AtomicOrdering.h"
#include "../../../../../llvm/include/llvm/Support/ConvertUTF.h"
#include "../../../../../llvm/include/llvm/Support/FileSystem/UniqueID.h"
#include "../../../../../llvm/include/llvm/Support/PrettyStackTrace.h"
#include "../../../../../llvm/include/llvm/Target/TargetOptions.h"
#include "../../../../../llvm/include/llvm/TargetParser/AtomicScope.h"

lldb_private::ExternalASTSourceWrapper::~ExternalASTSourceWrapper() = default;

void lldb_private::ExternalASTSourceWrapper::PrintStats() {
  m_Source->PrintStats();
}

lldb_private::ASTConsumerForwarder::~ASTConsumerForwarder() = default;

void lldb_private::ASTConsumerForwarder::PrintStats() { m_c->PrintStats(); }

lldb_private::SemaSourceWithPriorities::~SemaSourceWithPriorities() = default;

void lldb_private::SemaSourceWithPriorities::PrintStats() {
  for (size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->PrintStats();
}
