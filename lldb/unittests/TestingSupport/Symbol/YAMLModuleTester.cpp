//===-- YAMLModuleTester.cpp ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TestingSupport/Symbol/YAMLModuleTester.h"
#include "../../../../clang/include/clang/AST/ASTContextAllocate.h"
#include "../../../../clang/include/clang/AST/ASTFwd.h"
#include "../../../../clang/include/clang/AST/Attr.h"
#include "../../../../clang/include/clang/AST/CanonicalType.h"
#include "../../../../clang/include/clang/AST/CommentCommandTraits.h"
#include "../../../../clang/include/clang/AST/ComparisonCategories.h"
#include "../../../../clang/include/clang/AST/DeclBase.h"
#include "../../../../clang/include/clang/AST/DeclCXX.h"
#include "../../../../clang/include/clang/AST/DeclID.h"
#include "../../../../clang/include/clang/AST/Expr.h"
#include "../../../../clang/include/clang/AST/ExternalASTSource.h"
#include "../../../../clang/include/clang/AST/OperationKinds.h"
#include "../../../../clang/include/clang/AST/PrettyPrinter.h"
#include "../../../../clang/include/clang/AST/RawCommentList.h"
#include "../../../../clang/include/clang/AST/SYCLKernelInfo.h"
#include "../../../../clang/include/clang/AST/Stmt.h"
#include "../../../../clang/include/clang/AST/TypeBase.h"
#include "../../../../clang/include/clang/AST/TypeLoc.h"
#include "../../../../clang/include/clang/AST/TypeOrdering.h"
#include "../../../../clang/include/clang/Basic/AttrKinds.h"
#include "../../../../clang/include/clang/Basic/AttributeCommonInfo.h"
#include "../../../../clang/include/clang/Basic/BuiltinTraits.h"
#include "../../../../clang/include/clang/Basic/Builtins.h"
#include "../../../../clang/include/clang/Basic/CFProtectionOptions.h"
#include "../../../../clang/include/clang/Basic/CodeGenOptions.h"
#include "../../../../clang/include/clang/Basic/CustomizableOptional.h"
#include "../../../../clang/include/clang/Basic/DiagnosticCategories.h"
#include "../../../../clang/include/clang/Basic/DiagnosticIDs.h"
#include "../../../../clang/include/clang/Basic/DiagnosticOptions.h"
#include "../../../../clang/include/clang/Basic/DirectoryEntry.h"
#include "../../../../clang/include/clang/Basic/FileEntry.h"
#include "../../../../clang/include/clang/Basic/LangOptions.h"
#include "../../../../clang/include/clang/Basic/LangStandard.h"
#include "../../../../clang/include/clang/Basic/OpenCLOptions.h"
#include "../../../../clang/include/clang/Basic/OpenMPKinds.h"
#include "../../../../clang/include/clang/Basic/OperatorKinds.h"
#include "../../../../clang/include/clang/Basic/OptionalUnsigned.h"
#include "../../../../clang/include/clang/Basic/PragmaKinds.h"
#include "../../../../clang/include/clang/Basic/Sanitizers.h"
#include "../../../../clang/include/clang/Basic/TargetCXXABI.h"
#include "../../../../clang/include/clang/Basic/TargetInfo.h"
#include "../../../../clang/include/clang/Basic/TokenKinds.h"
#include "../../../../clang/include/clang/Lex/MacroBase.h"
#include "../../../../llvm/include/llvm/ADT/APFixedPoint.h"
#include "../../../../llvm/include/llvm/ADT/FunctionExtras.h"
#include "../../../../llvm/include/llvm/Frontend/HLSL/HLSLRootSignature.h"
#include "../../../../llvm/include/llvm/Support/ConvertUTF.h"
#include "../../../../llvm/include/llvm/Support/TypeSize.h"
#include "../../../../llvm/include/llvm/TargetParser/AtomicScope.h"
#include "Plugins/SymbolFile/DWARF/DWARFDebugInfo.h"
#include "llvm/ObjectYAML/DWARFEmitter.h"

using namespace lldb_private;
using namespace lldb_private::plugin::dwarf;

YAMLModuleTester::YAMLModuleTester(llvm::StringRef yaml_data, size_t cu_index) {
  llvm::Expected<TestFile> File = TestFile::fromYaml(yaml_data);
  EXPECT_THAT_EXPECTED(File, llvm::Succeeded());
  m_file = std::move(*File);

  m_module_sp = std::make_shared<Module>(m_file->moduleSpec());
  auto &symfile = *llvm::cast<SymbolFileDWARF>(m_module_sp->GetSymbolFile());

  m_dwarf_unit = symfile.DebugInfo().GetUnitAtIndex(cu_index);
}
