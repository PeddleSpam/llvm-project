//===-- StackFrameRecognizerTest.cpp --------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "lldb/Target/StackFrameRecognizer.h"
#include "../../../clang/include/clang/AST/ASTContextAllocate.h"
#include "../../../clang/include/clang/AST/ASTFwd.h"
#include "../../../clang/include/clang/AST/Attr.h"
#include "../../../clang/include/clang/AST/CanonicalType.h"
#include "../../../clang/include/clang/AST/CommentCommandTraits.h"
#include "../../../clang/include/clang/AST/ComparisonCategories.h"
#include "../../../clang/include/clang/AST/DeclBase.h"
#include "../../../clang/include/clang/AST/DeclCXX.h"
#include "../../../clang/include/clang/AST/Expr.h"
#include "../../../clang/include/clang/AST/ExternalASTSource.h"
#include "../../../clang/include/clang/AST/OperationKinds.h"
#include "../../../clang/include/clang/AST/PrettyPrinter.h"
#include "../../../clang/include/clang/AST/RawCommentList.h"
#include "../../../clang/include/clang/AST/SYCLKernelInfo.h"
#include "../../../clang/include/clang/AST/Stmt.h"
#include "../../../clang/include/clang/AST/TypeBase.h"
#include "../../../clang/include/clang/AST/TypeLoc.h"
#include "../../../clang/include/clang/AST/TypeOrdering.h"
#include "../../../clang/include/clang/Basic/AttrKinds.h"
#include "../../../clang/include/clang/Basic/AttributeCommonInfo.h"
#include "../../../clang/include/clang/Basic/BuiltinTraits.h"
#include "../../../clang/include/clang/Basic/Builtins.h"
#include "../../../clang/include/clang/Basic/CFProtectionOptions.h"
#include "../../../clang/include/clang/Basic/CodeGenOptions.h"
#include "../../../clang/include/clang/Basic/CustomizableOptional.h"
#include "../../../clang/include/clang/Basic/Diagnostic.h"
#include "../../../clang/include/clang/Basic/DiagnosticCategories.h"
#include "../../../clang/include/clang/Basic/DiagnosticIDs.h"
#include "../../../clang/include/clang/Basic/DiagnosticOptions.h"
#include "../../../clang/include/clang/Basic/DirectoryEntry.h"
#include "../../../clang/include/clang/Basic/FileEntry.h"
#include "../../../clang/include/clang/Basic/LangOptions.h"
#include "../../../clang/include/clang/Basic/LangStandard.h"
#include "../../../clang/include/clang/Basic/OpenCLOptions.h"
#include "../../../clang/include/clang/Basic/OpenMPKinds.h"
#include "../../../clang/include/clang/Basic/OperatorKinds.h"
#include "../../../clang/include/clang/Basic/PragmaKinds.h"
#include "../../../clang/include/clang/Basic/Sanitizers.h"
#include "../../../clang/include/clang/Basic/TargetCXXABI.h"
#include "../../../clang/include/clang/Basic/TargetInfo.h"
#include "../../../clang/include/clang/Basic/TokenKinds.h"
#include "../../../clang/include/clang/Lex/MacroBase.h"
#include "../../../llvm/include/llvm/ADT/APFixedPoint.h"
#include "../../../llvm/include/llvm/Frontend/HLSL/HLSLRootSignature.h"
#include "../../../llvm/include/llvm/Support/TypeSize.h"
#include "../../../llvm/include/llvm/TargetParser/AtomicScope.h"
#include "Plugins/Platform/Linux/PlatformLinux.h"
#include "lldb/Core/Debugger.h"
#include "lldb/Host/HostInfo.h"
#include "gtest/gtest.h"

using namespace lldb_private;
using namespace lldb;

namespace {
class StackFrameRecognizerTest : public ::testing::Test {
public:
  void SetUp() override {
    FileSystem::Initialize();
    HostInfo::Initialize();

    // Pretend Linux is the host platform.
    platform_linux::PlatformLinux::Initialize();
    ArchSpec arch("powerpc64-pc-linux");
    Platform::SetHostPlatform(
        platform_linux::PlatformLinux::CreateInstance(true, &arch));
  }

  void TearDown() override {
    platform_linux::PlatformLinux::Terminate();
    HostInfo::Terminate();
    FileSystem::Terminate();
  }
};

class DummyStackFrameRecognizer : public StackFrameRecognizer {
public:
  std::string GetName() override { return "Dummy StackFrame Recognizer"; }
};

void RegisterDummyStackFrameRecognizer(StackFrameRecognizerManager &manager) {
  RegularExpressionSP module_regex_sp = nullptr;
  RegularExpressionSP symbol_regex_sp(new RegularExpression("boom"));

  StackFrameRecognizerSP dummy_recognizer_sp(new DummyStackFrameRecognizer());

  manager.AddRecognizer(dummy_recognizer_sp, module_regex_sp, symbol_regex_sp,
                        Mangled::NamePreference::ePreferDemangled, false);
}

} // namespace

TEST_F(StackFrameRecognizerTest, NullModuleRegex) {
  DebuggerSP debugger_sp = Debugger::CreateInstance();
  ASSERT_TRUE(debugger_sp);

  StackFrameRecognizerManager manager;

  RegisterDummyStackFrameRecognizer(manager);

  bool any_printed = false;
  manager.ForEach([&any_printed](uint32_t recognizer_id, bool enabled,
                                 std::string name, std::string function,
                                 llvm::ArrayRef<ConstString> symbols,
                                 Mangled::NamePreference symbol_mangling,
                                 bool regexp) { any_printed = true; });

  EXPECT_TRUE(any_printed);
}
