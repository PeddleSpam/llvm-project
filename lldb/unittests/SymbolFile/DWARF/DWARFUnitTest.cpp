//===-- DWARFUnitTest.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

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
#include "TestingSupport/Symbol/YAMLModuleTester.h"

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::plugin::dwarf;

TEST(DWARFUnitTest, NullUnitDie) {
  // Make sure we don't crash parsing a null unit DIE.
  const char *yamldata = R"(
--- !ELF
FileHeader:
  Class:   ELFCLASS64
  Data:    ELFDATA2LSB
  Type:    ET_EXEC
  Machine: EM_386
DWARF:
  debug_abbrev:
    - Table:
        - Code:            0x00000001
          Tag:             DW_TAG_compile_unit
          Children:        DW_CHILDREN_yes
          Attributes:
            - Attribute:       DW_AT_language
              Form:            DW_FORM_data2
  debug_info:
    - Version:         4
      AddrSize:        8
      Entries:
        - AbbrCode:        0x00000000
)";

  YAMLModuleTester t(yamldata);
  ASSERT_TRUE((bool)t.GetDwarfUnit());

  DWARFUnit *unit = t.GetDwarfUnit();
  const DWARFDebugInfoEntry *die_first = unit->DIE().GetDIE();
  ASSERT_NE(die_first, nullptr);
  EXPECT_TRUE(die_first->IsNULL());
}

TEST(DWARFUnitTest, MissingSentinel) {
  // Make sure we don't crash if the debug info is missing a null DIE sentinel.
  const char *yamldata = R"(
--- !ELF
FileHeader:
  Class:   ELFCLASS64
  Data:    ELFDATA2LSB
  Type:    ET_EXEC
  Machine: EM_386
DWARF:
  debug_abbrev:
    - Table:
        - Code:            0x00000001
          Tag:             DW_TAG_compile_unit
          Children:        DW_CHILDREN_yes
          Attributes:
            - Attribute:       DW_AT_language
              Form:            DW_FORM_data2
  debug_info:
    - Version:         4
      AddrSize:        8
      Entries:
        - AbbrCode:        0x00000001
          Values:
            - Value:           0x000000000000000C
)";

  YAMLModuleTester t(yamldata);
  ASSERT_TRUE((bool)t.GetDwarfUnit());

  DWARFUnit *unit = t.GetDwarfUnit();
  const DWARFDebugInfoEntry *die_first = unit->DIE().GetDIE();
  ASSERT_NE(die_first, nullptr);
  EXPECT_EQ(die_first->GetFirstChild(), nullptr);
  EXPECT_EQ(die_first->GetSibling(), nullptr);
}

TEST(DWARFUnitTest, ClangProducer) {
  const char *yamldata = R"(
--- !ELF
FileHeader:
  Class:   ELFCLASS64
  Data:    ELFDATA2LSB
  Type:    ET_EXEC
  Machine: EM_386
DWARF:
  debug_str:
    - 'Apple clang version 13.0.0 (clang-1300.0.29.3)'
  debug_abbrev:
    - Table:
        - Code:            0x00000001
          Tag:             DW_TAG_compile_unit
          Children:        DW_CHILDREN_yes
          Attributes:
            - Attribute:       DW_AT_producer
              Form:            DW_FORM_strp
  debug_info:
    - Version:         4
      AddrSize:        8
      Entries:
        - AbbrCode:        0x1
          Values:
            - Value:           0x0
        - AbbrCode:        0x0
)";

  YAMLModuleTester t(yamldata);
  DWARFUnit *unit = t.GetDwarfUnit();
  ASSERT_TRUE((bool)unit);
  EXPECT_EQ(unit->GetProducer(), eProducerClang);
  EXPECT_EQ(unit->GetProducerVersion(), llvm::VersionTuple(1300, 0, 29, 3));
}

TEST(DWARFUnitTest, SwiftProducer) {
  const char *yamldata = R"(
--- !ELF
FileHeader:
  Class:   ELFCLASS64
  Data:    ELFDATA2LSB
  Type:    ET_EXEC
  Machine: EM_386
DWARF:
  debug_str:
    - 'Apple Swift version 5.5 (swiftlang-1300.0.31.1 clang-1300.0.29.1)'
  debug_abbrev:
    - Table:
        - Code:            0x00000001
          Tag:             DW_TAG_compile_unit
          Children:        DW_CHILDREN_yes
          Attributes:
            - Attribute:       DW_AT_producer
              Form:            DW_FORM_strp
  debug_info:
    - Version:         4
      AddrSize:        8
      Entries:
        - AbbrCode:        0x1
          Values:
            - Value:           0x0
        - AbbrCode:        0x0
)";

  YAMLModuleTester t(yamldata);
  DWARFUnit *unit = t.GetDwarfUnit();
  ASSERT_TRUE((bool)unit);
  EXPECT_EQ(unit->GetProducer(), eProducerSwift);
  EXPECT_EQ(unit->GetProducerVersion(), llvm::VersionTuple(1300, 0, 31, 1));
}

TEST(DWARFUnitTest, Swift5ComponentProducer) {
  const char *yamldata = R"(
--- !ELF
FileHeader:
  Class:   ELFCLASS64
  Data:    ELFDATA2LSB
  Type:    ET_EXEC
  Machine: EM_386
DWARF:
  debug_str:
    - 'Apple Swift version 6.2 (swiftlang-1.2.3.4.5 clang-1300.0.29.1)'
  debug_abbrev:
    - Table:
        - Code:            0x00000001
          Tag:             DW_TAG_compile_unit
          Children:        DW_CHILDREN_yes
          Attributes:
            - Attribute:       DW_AT_producer
              Form:            DW_FORM_strp
  debug_info:
    - Version:         4
      AddrSize:        8
      Entries:
        - AbbrCode:        0x1
          Values:
            - Value:           0x0
        - AbbrCode:        0x0
)";

  YAMLModuleTester t(yamldata);
  DWARFUnit *unit = t.GetDwarfUnit();
  ASSERT_TRUE((bool)unit);
  EXPECT_EQ(unit->GetProducer(), eProducerSwift);
  EXPECT_EQ(unit->GetProducerVersion(), llvm::VersionTuple(1, 2, 3, 4, 5));
}
