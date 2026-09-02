//===---DynamicValueObjectLocalBuffer.cpp-----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../../../clang/include/clang/AST/APNumericStorage.h"
#include "../../../clang/include/clang/AST/ASTContextAllocate.h"
#include "../../../clang/include/clang/AST/ASTFwd.h"
#include "../../../clang/include/clang/AST/ASTVector.h"
#include "../../../clang/include/clang/AST/Attr.h"
#include "../../../clang/include/clang/AST/CanonicalType.h"
#include "../../../clang/include/clang/AST/CharUnits.h"
#include "../../../clang/include/clang/AST/CommentCommandTraits.h"
#include "../../../clang/include/clang/AST/ComparisonCategories.h"
#include "../../../clang/include/clang/AST/ComputeDependence.h"
#include "../../../clang/include/clang/AST/DeclAccessPair.h"
#include "../../../clang/include/clang/AST/DeclBase.h"
#include "../../../clang/include/clang/AST/DeclCXX.h"
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
#include "../../../clang/include/clang/Basic/Builtins.h"
#include "../../../clang/include/clang/Basic/CFProtectionOptions.h"
#include "../../../clang/include/clang/Basic/CharInfo.h"
#include "../../../clang/include/clang/Basic/CodeGenOptions.h"
#include "../../../clang/include/clang/Basic/CustomizableOptional.h"
#include "../../../clang/include/clang/Basic/DiagnosticCategories.h"
#include "../../../clang/include/clang/Basic/DiagnosticIDs.h"
#include "../../../clang/include/clang/Basic/DiagnosticOptions.h"
#include "../../../clang/include/clang/Basic/DirectoryEntry.h"
#include "../../../clang/include/clang/Basic/FileEntry.h"
#include "../../../clang/include/clang/Basic/IdentifierTable.h"
#include "../../../clang/include/clang/Basic/LangOptions.h"
#include "../../../clang/include/clang/Basic/LangStandard.h"
#include "../../../clang/include/clang/Basic/OpenCLOptions.h"
#include "../../../clang/include/clang/Basic/OpenMPKinds.h"
#include "../../../clang/include/clang/Basic/OperatorKinds.h"
#include "../../../clang/include/clang/Basic/OptionalUnsigned.h"
#include "../../../clang/include/clang/Basic/PragmaKinds.h"
#include "../../../clang/include/clang/Basic/Sanitizers.h"
#include "../../../clang/include/clang/Basic/SyncScope.h"
#include "../../../clang/include/clang/Basic/TargetCXXABI.h"
#include "../../../clang/include/clang/Basic/TargetInfo.h"
#include "../../../clang/include/clang/Basic/TokenKinds.h"
#include "../../../clang/include/clang/Lex/MacroBase.h"
#include "../../../llvm/include/llvm/ADT/APFixedPoint.h"
#include "../../../llvm/include/llvm/Frontend/HLSL/HLSLRootSignature.h"
#include "../../../llvm/include/llvm/Support/AtomicOrdering.h"
#include "../../../llvm/include/llvm/Support/ConvertUTF.h"
#include "../../../llvm/include/llvm/Support/TypeSize.h"
#include "../../../llvm/include/llvm/TargetParser/AtomicScope.h"
#include "Plugins/Platform/Linux/PlatformLinux.h"
#include "Plugins/ScriptInterpreter/None/ScriptInterpreterNone.h"
#include "TestingSupport/SubsystemRAII.h"
#include "TestingSupport/Symbol/ClangTestUtils.h"
#include "lldb/Core/Debugger.h"
#include "lldb/Target/Language.h"
#include "lldb/Target/LanguageRuntime.h"
#include "lldb/ValueObject/ValueObjectConstResult.h"


using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::clang_utils;

// This entire class is boilerplate.
struct MockLanguage : public Language {

  llvm::StringRef GetPluginName() override { return "MockLanguage"; }
  lldb::LanguageType GetLanguageType() const override {
    return lldb::eLanguageTypeC_plus_plus;
  };

  static Language *CreateInstance(lldb::LanguageType language) {
    return new MockLanguage();
  }
  static void Initialize() {
    PluginManager::RegisterPlugin("MockLanguage", "Mock Language",
                                  CreateInstance);
  };

  static void Terminate() { PluginManager::UnregisterPlugin(CreateInstance); }
  bool IsSourceFile(llvm::StringRef file_path) const override { return true; }
};
LLDB_PLUGIN_DEFINE(MockLanguage)

struct MockLanguageRuntime : public LanguageRuntime {
  // This is the only method in this class that matters for this test.
  // This will unconditionally succeed and return a type with size 4,
  // a value_type of HostAddress, and a local buffer that points to the parent's
  // local buffer.
  // The tests will set that buffer to be either be larger or smaller than the
  // type we're returning.
  bool
  GetDynamicTypeAndAddress(ValueObject &in_value,
                           lldb::DynamicValueType use_dynamic,
                           TypeAndOrName &class_type_or_name, Address &address,
                           Value::ValueType &value_type,
                           llvm::ArrayRef<uint8_t> &local_buffer) override {
    auto ast = in_value.GetCompilerType().GetTypeSystem<TypeSystemClang>();

    auto int_type = createRecordWithField(
        *ast, "TypeWitInt", ast->GetBasicType(lldb::BasicType::eBasicTypeInt),
        "theIntField", LanguageType::eLanguageTypeC_plus_plus);
    class_type_or_name.SetCompilerType(int_type);
    local_buffer = in_value.GetLocalBuffer();
    value_type = Value::ValueType::HostAddress;
    return true;
  }

  // All of this is boilerplate.
  MockLanguageRuntime(Process *process) : LanguageRuntime(process) {}
  llvm::StringRef GetPluginName() override { return "MockLanguageRuntime"; }
  lldb::LanguageType GetLanguageType() const override {
    return lldb::eLanguageTypeC_plus_plus;
  }

  llvm::Error GetObjectDescription(Stream &str, ValueObject &object) override {
    return llvm::Error::success();
  }

  llvm::Error GetObjectDescription(Stream &str, Value &value,
                                   ExecutionContextScope *exe_scope) override {
    return llvm::Error::success();
  }

  bool CouldHaveDynamicValue(ValueObject &in_value) override { return true; }

  TypeAndOrName FixUpDynamicType(const TypeAndOrName &type_and_or_name,
                                 ValueObject &static_value) override {
    return type_and_or_name;
  }

  lldb::BreakpointResolverSP
  CreateExceptionResolver(const lldb::BreakpointSP &bkpt, bool catch_bp,
                          bool throw_bp) override {
    return lldb::BreakpointResolverSP();
  }

  lldb::ThreadPlanSP GetStepThroughTrampolinePlan(Thread &thread,
                                                  bool stop_others) override {
    return {};
  }

  static LanguageRuntime *CreateInstance(Process *process,
                                         LanguageType language) {
    return new MockLanguageRuntime(process);
  }

  static void Initialize() {
    PluginManager::RegisterPlugin(
        "MockLanguageRuntime", "MockLanguageRuntime", CreateInstance,
        [](CommandInterpreter &interpreter) -> lldb::CommandObjectSP {
          return {};
        },
        [](lldb::LanguageType language,
           bool throw_bp) -> BreakpointPreconditionSP { return {}; });
  }

  static void Terminate() { PluginManager::UnregisterPlugin(CreateInstance); }
};
LLDB_PLUGIN_DEFINE(MockLanguageRuntime)

// This entire class is boilerplate.
struct MockProcess : Process {
  MockProcess(lldb::TargetSP target_sp, lldb::ListenerSP listener_sp)
      : Process(target_sp, listener_sp) {}

  llvm::StringRef GetPluginName() override { return "mock process"; }

  bool CanDebug(lldb::TargetSP target, bool plugin_specified_by_name) override {
    return false;
  };

  Status DoDestroy() override { return {}; }

  void RefreshStateAfterStop() override {}

  bool DoUpdateThreadList(ThreadList &old_thread_list,
                          ThreadList &new_thread_list) override {
    return false;
  };

  size_t DoReadMemory(const ProcessAddress &process_addr, void *buf,
                      size_t size, Status &error) override {
    // No need to read memory in these tests.
    return size;
  }
};

class DynamicValueObjectLocalBufferTest : public ::testing::Test {
public:
  void SetUp() override {
    ArchSpec arch("i386-pc-linux");
    Platform::SetHostPlatform(
        platform_linux::PlatformLinux::CreateInstance(true, &arch));
    // std::call_once(TestUtilities::g_debugger_initialize_flag,
    //                []() { Debugger::Initialize(nullptr); });
    m_debugger_sp = Debugger::CreateInstance();
    ASSERT_TRUE(m_debugger_sp);
    m_debugger_sp->GetTargetList().CreateTarget(*m_debugger_sp, "", arch,
                                                eLoadDependentsNo,
                                                m_platform_sp, m_target_sp);
    ASSERT_TRUE(m_target_sp);
    ASSERT_TRUE(m_target_sp->GetArchitecture().IsValid());
    ASSERT_TRUE(m_platform_sp);
    m_listener_sp = Listener::MakeListener("dummy");
    m_process_sp = std::make_shared<MockProcess>(m_target_sp, m_listener_sp);
    ASSERT_TRUE(m_process_sp);
    m_exe_ctx = ExecutionContext(m_process_sp);

    m_holder = std::make_unique<clang_utils::TypeSystemClangHolder>("test");
    m_type_system = m_holder->GetAST();
    LLDB_PLUGIN_INITIALIZE(MockLanguage);
    LLDB_PLUGIN_INITIALIZE(MockLanguageRuntime);
  }
  void TearDown() override {
    LLDB_PLUGIN_TERMINATE(MockLanguage);
    LLDB_PLUGIN_TERMINATE(MockLanguageRuntime);
  }

  void TestValueObjectWithLocalBuffer(DataExtractor &data_extractor,
                                      bool should_succeed) {
    std::unique_ptr<TypeSystemClangHolder> holder =
        std::make_unique<TypeSystemClangHolder>("test ASTContext");
    TypeSystemClang *ast = holder->GetAST();
    auto char_type = createRecordWithField(
        *ast, "TypeWithChar",
        ast->GetBasicType(lldb::BasicType::eBasicTypeChar), "theField");

    ExecutionContextScope *exe_scope = m_exe_ctx.GetBestExecutionContextScope();
    ConstString var_name("test_var");
    auto valobj_sp = ValueObjectConstResult::Create(exe_scope, char_type,
                                                    var_name, data_extractor);
    auto dyn_valobj = valobj_sp->GetDynamicValue(lldb::eDynamicCanRunTarget);
    ASSERT_TRUE(dyn_valobj->GetValueIsValid() == should_succeed);
  }

  SubsystemRAII<FileSystem, HostInfo, platform_linux::PlatformLinux,
                ScriptInterpreterNone>
      m_subsystems;
  std::unique_ptr<clang_utils::TypeSystemClangHolder> m_holder;
  lldb::DebuggerSP m_debugger_sp;
  lldb::TargetSP m_target_sp;
  lldb::PlatformSP m_platform_sp;
  lldb::ListenerSP m_listener_sp;
  lldb::ProcessSP m_process_sp;
  ExecutionContext m_exe_ctx;
  TypeSystemClang *m_type_system;
};

TEST_F(DynamicValueObjectLocalBufferTest, BufferTooSmall) {
  /// Test that a value object with a buffer to small to fit the
  /// "dynamic" type will return an invalid dynamic value object.
  uint8_t value = 1;
  ByteOrder endian = endian::InlHostByteOrder();
  DataExtractor data_extractor{&value, sizeof(value), endian, 4};
  TestValueObjectWithLocalBuffer(data_extractor, false);
}

TEST_F(DynamicValueObjectLocalBufferTest, BufferTooBig) {
  /// Test that a value object with a buffer big enough fit the
  /// "dynamic" type will return a valid dynamic value object.
  uint64_t value = 1;
  ByteOrder endian = endian::InlHostByteOrder();
  DataExtractor data_extractor{&value, sizeof(value), endian, 4};
  TestValueObjectWithLocalBuffer(data_extractor, true);
}

TEST_F(DynamicValueObjectLocalBufferTest, BufferExactlyRight) {
  /// Test that a value object with a buffer exactly the size of the
  /// "dynamic" type will return a valid dynamic value object.
  uint32_t value = 1;
  ByteOrder endian = endian::InlHostByteOrder();
  DataExtractor data_extractor{&value, sizeof(value), endian, 4};
  TestValueObjectWithLocalBuffer(data_extractor, true);
}
