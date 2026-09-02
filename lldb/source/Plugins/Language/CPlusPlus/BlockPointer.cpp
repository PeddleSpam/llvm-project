//===-- BlockPointer.cpp --------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "BlockPointer.h"

#include "../../../../../clang/include/clang/APINotes/APINotesManager.h"
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
#include "../../../../../clang/include/clang/AST/DeclObjCCommon.h"
#include "../../../../../clang/include/clang/AST/Expr.h"
#include "../../../../../clang/include/clang/AST/ExternalASTSource.h"
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
#include "../../../../../clang/include/clang/Basic/BuiltinTraits.h"
#include "../../../../../clang/include/clang/Basic/Builtins.h"
#include "../../../../../clang/include/clang/Basic/CFProtectionOptions.h"
#include "../../../../../clang/include/clang/Basic/CodeGenOptions.h"
#include "../../../../../clang/include/clang/Basic/CustomizableOptional.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticCategories.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticIDs.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticOptions.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticSema.h"
#include "../../../../../clang/include/clang/Basic/DirectoryEntry.h"
#include "../../../../../clang/include/clang/Basic/FileEntry.h"
#include "../../../../../clang/include/clang/Basic/LangOptions.h"
#include "../../../../../clang/include/clang/Basic/LangStandard.h"
#include "../../../../../clang/include/clang/Basic/OffloadArch.h"
#include "../../../../../clang/include/clang/Basic/OpenMPKinds.h"
#include "../../../../../clang/include/clang/Basic/OperatorKinds.h"
#include "../../../../../clang/include/clang/Basic/OptionalUnsigned.h"
#include "../../../../../clang/include/clang/Basic/ParsedAttrInfo.h"
#include "../../../../../clang/include/clang/Basic/Sanitizers.h"
#include "../../../../../clang/include/clang/Basic/TargetCXXABI.h"
#include "../../../../../clang/include/clang/Basic/TargetInfo.h"
#include "../../../../../clang/include/clang/Basic/TokenKinds.h"
#include "../../../../../clang/include/clang/Lex/MacroBase.h"
#include "../../../../../clang/include/clang/Lex/Token.h"
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
#include "Plugins/ExpressionParser/Clang/ClangASTImporter.h"
#include "Plugins/TypeSystem/Clang/TypeSystemClang.h"
#include "lldb/DataFormatters/FormattersHelpers.h"
#include "lldb/ValueObject/ValueObjectConstResult.h"

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::formatters;

namespace lldb_private {
namespace formatters {

class BlockPointerSyntheticFrontEnd : public SyntheticChildrenFrontEnd {
public:
  BlockPointerSyntheticFrontEnd(lldb::ValueObjectSP valobj_sp)
      : SyntheticChildrenFrontEnd(*valobj_sp), m_block_struct_type() {
    CompilerType block_pointer_type(m_backend.GetCompilerType());
    CompilerType function_pointer_type;
    block_pointer_type.IsBlockPointerType(&function_pointer_type);

    TargetSP target_sp(m_backend.GetTargetSP());

    if (!target_sp) {
      return;
    }

    auto type_system_or_err = target_sp->GetScratchTypeSystemForLanguage(
        lldb::eLanguageTypeC_plus_plus);
    if (auto err = type_system_or_err.takeError()) {
      LLDB_LOG_ERROR(GetLog(LLDBLog::DataFormatters), std::move(err),
                     "Failed to get scratch TypeSystemClang: {0}");
      return;
    }

    auto clang_ast_context =
        block_pointer_type.GetTypeSystem<TypeSystemClang>();
    if (!clang_ast_context)
      return;

    const char *const isa_name("__isa");
    const CompilerType isa_type =
        clang_ast_context->GetBasicType(lldb::eBasicTypeObjCClass);
    const char *const flags_name("__flags");
    const CompilerType flags_type =
        clang_ast_context->GetBasicType(lldb::eBasicTypeInt);
    const char *const reserved_name("__reserved");
    const CompilerType reserved_type =
        clang_ast_context->GetBasicType(lldb::eBasicTypeInt);
    const char *const FuncPtr_name("__FuncPtr");

    m_block_struct_type = clang_ast_context->CreateStructForIdentifier(
        llvm::StringRef(), {{isa_name, isa_type},
                            {flags_name, flags_type},
                            {reserved_name, reserved_type},
                            {FuncPtr_name, function_pointer_type}});
  }

  ~BlockPointerSyntheticFrontEnd() override = default;

  llvm::Expected<uint32_t> CalculateNumChildren() override {
    const bool omit_empty_base_classes = false;
    return m_block_struct_type.GetNumChildren(omit_empty_base_classes, nullptr);
  }

  lldb::ValueObjectSP GetChildAtIndex(uint32_t idx) override {
    if (!m_block_struct_type.IsValid()) {
      return lldb::ValueObjectSP();
    }

    if (idx >= CalculateNumChildrenIgnoringErrors()) {
      return lldb::ValueObjectSP();
    }

    const bool thread_and_frame_only_if_stopped = true;
    ExecutionContext exe_ctx = m_backend.GetExecutionContextRef().Lock(
        thread_and_frame_only_if_stopped);
    const bool transparent_pointers = false;
    const bool omit_empty_base_classes = false;
    const bool ignore_array_bounds = false;
    ValueObject *value_object = nullptr;

    std::string child_name;
    uint32_t child_byte_size = 0;
    int32_t child_byte_offset = 0;
    uint32_t child_bitfield_bit_size = 0;
    uint32_t child_bitfield_bit_offset = 0;
    bool child_is_base_class = false;
    bool child_is_deref_of_parent = false;
    uint64_t language_flags = 0;

    auto child_type_or_err = m_block_struct_type.GetChildCompilerTypeAtIndex(
        &exe_ctx, idx, transparent_pointers, omit_empty_base_classes,
        ignore_array_bounds, child_name, child_byte_size, child_byte_offset,
        child_bitfield_bit_size, child_bitfield_bit_offset, child_is_base_class,
        child_is_deref_of_parent, value_object, language_flags);
    if (!child_type_or_err)
      return ValueObjectConstResult::Create(
          exe_ctx.GetBestExecutionContextScope(),
          Status::FromError(child_type_or_err.takeError()));
    CompilerType child_type = *child_type_or_err;

    ValueObjectSP struct_pointer_sp =
        m_backend.Cast(m_block_struct_type.GetPointerType());

    if (!struct_pointer_sp) {
      return lldb::ValueObjectSP();
    }

    Status err;
    ValueObjectSP struct_sp = struct_pointer_sp->Dereference(err);

    if (!struct_sp || !err.Success()) {
      return lldb::ValueObjectSP();
    }

    ValueObjectSP child_sp(struct_sp->GetSyntheticChildAtOffset(
        child_byte_offset, child_type, true, ConstString(child_name)));

    return child_sp;
  }

  // return true if this object is now safe to use forever without ever
  // updating again; the typical (and tested) answer here is 'false'
  lldb::ChildCacheState Update() override {
    return lldb::ChildCacheState::eRefetch;
  }

  llvm::Expected<size_t> GetIndexOfChildWithName(ConstString name) override {
    if (!m_block_struct_type.IsValid())
      return llvm::createStringErrorV("type has no child named '{0}'", name);

    const bool omit_empty_base_classes = false;
    return m_block_struct_type.GetIndexOfChildWithName(name.AsCString(nullptr),
                                                       omit_empty_base_classes);
  }

private:
  CompilerType m_block_struct_type;
};

} // namespace formatters
} // namespace lldb_private

bool lldb_private::formatters::BlockPointerSummaryProvider(
    ValueObject &valobj, Stream &s, const TypeSummaryOptions &) {
  lldb_private::SyntheticChildrenFrontEnd *synthetic_children =
      BlockPointerSyntheticFrontEndCreator(nullptr, valobj.GetSP());
  if (!synthetic_children) {
    return false;
  }

  synthetic_children->Update();

  static const ConstString s_FuncPtr_name("__FuncPtr");

  auto index_or_err =
      synthetic_children->GetIndexOfChildWithName(s_FuncPtr_name);

  if (!index_or_err) {
    LLDB_LOG_ERROR(GetLog(LLDBLog::DataFormatters), index_or_err.takeError(),
                   "{0}");
    return false;
  }

  lldb::ValueObjectSP child_sp =
      synthetic_children->GetChildAtIndex(*index_or_err);

  if (!child_sp) {
    return false;
  }

  lldb::ValueObjectSP qualified_child_representation_sp =
      child_sp->GetQualifiedRepresentationIfAvailable(
          lldb::eDynamicDontRunTarget, true);

  const char *child_value =
      qualified_child_representation_sp->GetValueAsCString();

  s.Printf("%s", child_value);

  return true;
}

lldb_private::SyntheticChildrenFrontEnd *
lldb_private::formatters::BlockPointerSyntheticFrontEndCreator(
    CXXSyntheticChildren *, lldb::ValueObjectSP valobj_sp) {
  if (!valobj_sp)
    return nullptr;
  return new BlockPointerSyntheticFrontEnd(valobj_sp);
}
