//===-- ClangExpressionVariable.cpp ---------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ClangExpressionVariable.h"

#include "../../../../../clang/include/clang/AST/ASTConcept.h"
#include "../../../../../clang/include/clang/AST/ASTContextAllocate.h"
#include "../../../../../clang/include/clang/AST/ASTFwd.h"
#include "../../../../../clang/include/clang/AST/CommentCommandTraits.h"
#include "../../../../../clang/include/clang/AST/ComparisonCategories.h"
#include "../../../../../clang/include/clang/AST/DeclBase.h"
#include "../../../../../clang/include/clang/AST/DeclCXX.h"
#include "../../../../../clang/include/clang/AST/Expr.h"
#include "../../../../../clang/include/clang/AST/ExternalASTSource.h"
#include "../../../../../clang/include/clang/AST/NestedNameSpecifierBase.h"
#include "../../../../../clang/include/clang/AST/OperationKinds.h"
#include "../../../../../clang/include/clang/AST/PrettyPrinter.h"
#include "../../../../../clang/include/clang/AST/RawCommentList.h"
#include "../../../../../clang/include/clang/AST/SYCLKernelInfo.h"
#include "../../../../../clang/include/clang/AST/Stmt.h"
#include "../../../../../clang/include/clang/AST/TypeBase.h"
#include "../../../../../clang/include/clang/AST/TypeLoc.h"
#include "../../../../../clang/include/clang/AST/TypeOrdering.h"
#include "../../../../../clang/include/clang/Basic/AddressSpaces.h"
#include "../../../../../clang/include/clang/Basic/AttrKinds.h"
#include "../../../../../clang/include/clang/Basic/BitmaskEnum.h"
#include "../../../../../clang/include/clang/Basic/BuiltinTraits.h"
#include "../../../../../clang/include/clang/Basic/Builtins.h"
#include "../../../../../clang/include/clang/Basic/CFProtectionOptions.h"
#include "../../../../../clang/include/clang/Basic/CustomizableOptional.h"
#include "../../../../../clang/include/clang/Basic/Diagnostic.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticCategories.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticIDs.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticOptions.h"
#include "../../../../../clang/include/clang/Basic/DirectoryEntry.h"
#include "../../../../../clang/include/clang/Basic/FileEntry.h"
#include "../../../../../clang/include/clang/Basic/LangOptions.h"
#include "../../../../../clang/include/clang/Basic/LangStandard.h"
#include "../../../../../clang/include/clang/Basic/OperatorKinds.h"
#include "../../../../../clang/include/clang/Basic/PragmaKinds.h"
#include "../../../../../clang/include/clang/Basic/Sanitizers.h"
#include "../../../../../clang/include/clang/Basic/SyncScope.h"
#include "../../../../../clang/include/clang/Basic/TargetCXXABI.h"
#include "../../../../../clang/include/clang/Basic/TokenKinds.h"
#include "../../../../../clang/include/clang/Lex/MacroBase.h"
#include "../../../../../llvm/include/llvm/ADT/APFixedPoint.h"
#include "../../../../../llvm/include/llvm/Frontend/HLSL/HLSLRootSignature.h"
#include "../../../../../llvm/include/llvm/Support/TypeSize.h"
#include "../../../../../llvm/include/llvm/Target/TargetOptions.h"
#include "lldb/ValueObject/ValueObjectConstResult.h"
#include "clang/AST/ASTContext.h"

using namespace lldb_private;
using namespace clang;

char ClangExpressionVariable::ID;

ClangExpressionVariable::ClangExpressionVariable(
    ExecutionContextScope *exe_scope, lldb::ByteOrder byte_order,
    uint32_t addr_byte_size)
    : m_parser_vars(), m_jit_vars() {
  m_flags = EVNone;
  m_frozen_sp =
      ValueObjectConstResult::Create(exe_scope, byte_order, addr_byte_size);
}

ClangExpressionVariable::ClangExpressionVariable(
    ExecutionContextScope *exe_scope, Value &value, ConstString name,
    uint16_t flags)
    : m_parser_vars(), m_jit_vars() {
  m_flags = flags;
  m_frozen_sp = ValueObjectConstResult::Create(exe_scope, value, name);
}

ClangExpressionVariable::ClangExpressionVariable(
    const lldb::ValueObjectSP &valobj_sp)
    : m_parser_vars(), m_jit_vars() {
  m_flags = EVNone;
  m_frozen_sp = valobj_sp;
}

ClangExpressionVariable::ClangExpressionVariable(
    ExecutionContextScope *exe_scope, ConstString name,
    const TypeFromUser &user_type, lldb::ByteOrder byte_order,
    uint32_t addr_byte_size)
    : m_parser_vars(), m_jit_vars() {
  m_flags = EVNone;
  m_frozen_sp =
      ValueObjectConstResult::Create(exe_scope, byte_order, addr_byte_size);
  SetName(name);
  SetCompilerType(user_type);
}

TypeFromUser ClangExpressionVariable::GetTypeFromUser() {
  TypeFromUser tfu(GetValueObject()->GetCompilerType());
  return tfu;
}
