//===-- ProcessTraceTest.cpp ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "lldb/Target/ProcessTrace.h"
#include "../../../clang/include/clang/AST/ASTContextAllocate.h"
#include "../../../clang/include/clang/AST/ASTFwd.h"
#include "../../../clang/include/clang/AST/Attr.h"
#include "../../../clang/include/clang/AST/CanonicalType.h"
#include "../../../clang/include/clang/AST/CommentCommandTraits.h"
#include "../../../clang/include/clang/AST/ComparisonCategories.h"
#include "../../../clang/include/clang/AST/DeclBase.h"
#include "../../../clang/include/clang/AST/DeclCXX.h"
#include "../../../clang/include/clang/AST/DeclID.h"
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
using namespace platform_linux;

// This is needed for the tests that create a trace process.
class ProcessTraceTest : public ::testing::Test {
public:
  void SetUp() override {
    ProcessTrace::Initialize();
    FileSystem::Initialize();
    HostInfo::Initialize();
    PlatformLinux::Initialize();
  }
  void TearDown() override {
    PlatformLinux::Terminate();
    HostInfo::Terminate();
    FileSystem::Terminate();
    ProcessTrace::Terminate();
  }
};

TargetSP CreateTarget(DebuggerSP &debugger_sp, const ArchSpec &arch) {
  PlatformSP platform_sp;
  TargetSP target_sp;
  debugger_sp->GetTargetList().CreateTarget(
      *debugger_sp, "", arch, eLoadDependentsNo, platform_sp, target_sp);
  return target_sp;
}

// Test that we can create a process trace with a nullptr core file.
TEST_F(ProcessTraceTest, ConstructorWithNullptrCoreFile) {
  ArchSpec arch("i386-pc-linux");

  Platform::SetHostPlatform(PlatformLinux::CreateInstance(true, &arch));
  ASSERT_NE(Platform::GetHostPlatform(), nullptr);

  DebuggerSP debugger_sp = Debugger::CreateInstance();
  ASSERT_TRUE(debugger_sp);

  TargetSP target_sp = CreateTarget(debugger_sp, arch);
  ASSERT_TRUE(target_sp);

  ProcessSP process_sp = target_sp->CreateProcess(
      /*listener*/ nullptr, "trace",
      /*crash_file*/ nullptr,
      /*can_connect*/ false);

  ASSERT_NE(process_sp, nullptr);
}

// Test that a trace process stops after attaching.
TEST_F(ProcessTraceTest, DidAttach) {
  ArchSpec arch("i386-pc-linux");

  Platform::SetHostPlatform(PlatformLinux::CreateInstance(true, &arch));
  ASSERT_NE(Platform::GetHostPlatform(), nullptr);

  DebuggerSP debugger_sp = Debugger::CreateInstance();
  ASSERT_TRUE(debugger_sp);

  TargetSP target_sp = CreateTarget(debugger_sp, arch);
  ASSERT_TRUE(target_sp);

  ProcessSP process_sp = target_sp->CreateProcess(
      /*listener*/ nullptr, "trace",
      /*crash_file*/ nullptr,
      /*can_connect*/ false);

  ASSERT_NE(process_sp, nullptr);

  process_sp->DidAttach(arch);

  ASSERT_EQ(process_sp->GetState(), lldb::eStateStopped);
}
