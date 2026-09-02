//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CommonABIRuntime.h"

#include "../../../../../clang/include/clang/AST/ASTContextAllocate.h"
#include "../../../../../clang/include/clang/AST/ASTFwd.h"
#include "../../../../../clang/include/clang/AST/Attr.h"
#include "../../../../../clang/include/clang/AST/CanonicalType.h"
#include "../../../../../clang/include/clang/AST/CharUnits.h"
#include "../../../../../clang/include/clang/AST/CommentCommandTraits.h"
#include "../../../../../clang/include/clang/AST/ComparisonCategories.h"
#include "../../../../../clang/include/clang/AST/DeclBase.h"
#include "../../../../../clang/include/clang/AST/DeclCXX.h"
#include "../../../../../clang/include/clang/AST/DeclID.h"
#include "../../../../../clang/include/clang/AST/Expr.h"
#include "../../../../../clang/include/clang/AST/OperationKinds.h"
#include "../../../../../clang/include/clang/AST/PrettyPrinter.h"
#include "../../../../../clang/include/clang/AST/RawCommentList.h"
#include "../../../../../clang/include/clang/AST/SYCLKernelInfo.h"
#include "../../../../../clang/include/clang/AST/Stmt.h"
#include "../../../../../clang/include/clang/AST/TypeBase.h"
#include "../../../../../clang/include/clang/AST/TypeLoc.h"
#include "../../../../../clang/include/clang/AST/TypeOrdering.h"
#include "../../../../../clang/include/clang/Basic/AttrKinds.h"
#include "../../../../../clang/include/clang/Basic/AttributeCommonInfo.h"
#include "../../../../../clang/include/clang/Basic/BuiltinTraits.h"
#include "../../../../../clang/include/clang/Basic/Builtins.h"
#include "../../../../../clang/include/clang/Basic/CFProtectionOptions.h"
#include "../../../../../clang/include/clang/Basic/CodeGenOptions.h"
#include "../../../../../clang/include/clang/Basic/CustomizableOptional.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticCategories.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticIDs.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticOptions.h"
#include "../../../../../clang/include/clang/Basic/DirectoryEntry.h"
#include "../../../../../clang/include/clang/Basic/FileEntry.h"
#include "../../../../../clang/include/clang/Basic/LangOptions.h"
#include "../../../../../clang/include/clang/Basic/LangStandard.h"
#include "../../../../../clang/include/clang/Basic/OpenCLOptions.h"
#include "../../../../../clang/include/clang/Basic/OpenMPKinds.h"
#include "../../../../../clang/include/clang/Basic/OperatorKinds.h"
#include "../../../../../clang/include/clang/Basic/OptionalUnsigned.h"
#include "../../../../../clang/include/clang/Basic/PragmaKinds.h"
#include "../../../../../clang/include/clang/Basic/Sanitizers.h"
#include "../../../../../clang/include/clang/Basic/TargetCXXABI.h"
#include "../../../../../clang/include/clang/Basic/TargetInfo.h"
#include "../../../../../clang/include/clang/Basic/TokenKinds.h"
#include "../../../../../clang/include/clang/Lex/MacroBase.h"
#include "../../../../../llvm/include/llvm/ADT/APFixedPoint.h"
#include "../../../../../llvm/include/llvm/ADT/FunctionExtras.h"
#include "../../../../../llvm/include/llvm/Frontend/HLSL/HLSLRootSignature.h"
#include "../../../../../llvm/include/llvm/Support/ConvertUTF.h"
#include "../../../../../llvm/include/llvm/Support/TypeSize.h"
#include "../../../../../llvm/include/llvm/TargetParser/AtomicScope.h"
#include "Plugins/TypeSystem/Clang/TypeSystemClang.h"

using namespace lldb;
using namespace lldb_private;

CommonABIRuntime::CommonABIRuntime(Process *process) : m_process(process) {}

lldb::TypeSP CommonABIRuntime::LookupTypeByName(llvm::StringRef type_name,
                                                lldb::ModuleSP preferred_module,
                                                bool &any_found) const {
  Log *log = GetLog(LLDBLog::Object);

  any_found = false;

  ConstString const_lookup_name(type_name);
  TypeList class_types;
  // First look in the module that the vtable symbol came from and
  // look for a single exact match.
  TypeResults results;
  TypeQuery query(const_lookup_name.GetStringRef(),
                  TypeQueryOptions::e_exact_match |
                      TypeQueryOptions::e_strict_namespaces |
                      TypeQueryOptions::e_find_one);
  if (preferred_module) {
    preferred_module->FindTypes(query, results);
    TypeSP type_sp = results.GetFirstType();
    if (type_sp)
      class_types.Insert(type_sp);
  }

  // If we didn't find a symbol, then move on to the entire module
  // list in the target and get as many unique matches as possible
  if (class_types.Empty()) {
    query.SetFindOne(false);
    m_process->GetTarget().GetImages().FindTypes(nullptr, query, results);
    for (const auto &type_sp : results.GetTypeMap().Types())
      class_types.Insert(type_sp);
  }

  lldb::TypeSP type_sp;
  if (class_types.Empty()) {
    LLDB_LOG(log, "Failed to find '{0}'", type_name);
    return {};
  }
  any_found = true;

  if (class_types.GetSize() == 1) {
    type_sp = class_types.GetTypeAtIndex(0);
    if (!type_sp)
      return {};
    if (!TypeSystemClang::IsCXXClassType(type_sp->GetForwardCompilerType()))
      return {};

    return type_sp;
  }

  if (log) {
    LLDB_LOG(log,
             "'{0}' has multiple matching dynamic "
             "types:",
             type_name);
    for (size_t i = 0; i < class_types.GetSize(); i++) {
      type_sp = class_types.GetTypeAtIndex(i);
      if (type_sp) {
        LLDB_LOG(log, "[{0}]: uid={1:x}, type-name='{2}'", i, type_sp->GetID(),
                 type_sp->GetName());
      }
    }
  }

  for (size_t i = 0; i < class_types.GetSize(); i++) {
    type_sp = class_types.GetTypeAtIndex(i);
    if (type_sp) {
      if (TypeSystemClang::IsCXXClassType(type_sp->GetForwardCompilerType())) {
        LLDB_LOG(log,
                 "'{0}' has multiple matching dynamic types, "
                 "picking this one: [{1}] uid={2:x}, type-name='{3}'\n",
                 type_name, i, type_sp->GetID(), type_sp->GetName());
        return type_sp;
      }
    }
  }

  LLDB_LOG(log,
           "'{0}' has multiple matching dynamic types, didn't find a C++ match",
           type_name);
  return {};
}

TypeAndOrName
CommonABIRuntime::GetDynamicTypeInfo(const lldb_private::Address &vtable_addr) {
  std::lock_guard<std::mutex> locker(m_mutex);
  DynamicTypeCache::const_iterator pos = m_dynamic_type_map.find(vtable_addr);
  if (pos == m_dynamic_type_map.end())
    return TypeAndOrName();

  return pos->second;
}

void CommonABIRuntime::SetDynamicTypeInfo(
    const lldb_private::Address &vtable_addr, const TypeAndOrName &type_info) {
  std::lock_guard<std::mutex> locker(m_mutex);
  m_dynamic_type_map[vtable_addr] = type_info;
}
