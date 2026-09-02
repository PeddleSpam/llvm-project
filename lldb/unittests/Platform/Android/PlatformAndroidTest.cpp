//===-- PlatformAndroidTest.cpp -------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Plugins/Platform/Android/PlatformAndroid.h"
#include "../../../../clang/include/clang/AST/ASTContextAllocate.h"
#include "../../../../clang/include/clang/AST/ASTFwd.h"
#include "../../../../clang/include/clang/AST/Attr.h"
#include "../../../../clang/include/clang/AST/CanonicalType.h"
#include "../../../../clang/include/clang/AST/CommentCommandTraits.h"
#include "../../../../clang/include/clang/AST/ComparisonCategories.h"
#include "../../../../clang/include/clang/AST/DeclBase.h"
#include "../../../../clang/include/clang/AST/DeclCXX.h"
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
#include "Plugins/Platform/Android/PlatformAndroidRemoteGDBServer.h"
#include "lldb/Utility/Connection.h"
#include "gmock/gmock.h"

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::platform_android;
using namespace testing;

namespace {

class MockAdbClient : public AdbClient {
public:
  explicit MockAdbClient() : AdbClient() {}

  MOCK_METHOD3(ShellToFile,
               Status(const char *command, std::chrono::milliseconds timeout,
                      const FileSpec &output_file_spec));
};

class PlatformAndroidTest : public PlatformAndroid, public ::testing::Test {
public:
  PlatformAndroidTest() : PlatformAndroid(false) {
    m_remote_platform_sp = PlatformSP(new PlatformAndroidRemoteGDBServer());

    // Set up default mock behavior to avoid uninteresting call warnings
    ON_CALL(*this, GetSyncService(_))
        .WillByDefault([](Status &error) -> std::unique_ptr<AdbSyncService> {
          error = Status::FromErrorString("Sync service unavailable");
          return nullptr;
        });
  }

  MOCK_METHOD1(GetAdbClient, AdbClientUP(Status &error));
  MOCK_METHOD0(GetPropertyPackageName, llvm::StringRef());
  MOCK_METHOD1(GetSyncService, std::unique_ptr<AdbSyncService>(Status &error));

  // Make GetSyncService public for testing
  using PlatformAndroid::GetSyncService;
};

} // namespace

TEST_F(PlatformAndroidTest,
       DownloadModuleSlice_AdbClientError_FailsGracefully) {
  EXPECT_CALL(*this, GetAdbClient(_))
      .WillOnce(DoAll(WithArg<0>([](auto &arg) {
                        arg = Status::FromErrorString(
                            "Failed to create AdbClient");
                      }),
                      Return(ByMove(AdbClientUP()))));

  Status result = DownloadModuleSlice(
      FileSpec("/system/app/Test/Test.apk!/lib/arm64-v8a/libtest.so"), 4096,
      3600, FileSpec("/tmp/libtest.so"));

  EXPECT_TRUE(result.Fail());
  EXPECT_THAT(result.AsCString(), HasSubstr("Failed to create AdbClient"));
}

TEST_F(PlatformAndroidTest, DownloadModuleSlice_ZipFile_UsesCorrectDdCommand) {
  auto *adb_client = new MockAdbClient();
  EXPECT_CALL(*adb_client,
              ShellToFile(StrEq("dd if='/system/app/Test/Test.apk' "
                                "iflag=skip_bytes,count_bytes "
                                "skip=4096 count=3600 status=none"),
                          _, _))
      .WillOnce(Return(Status()));

  EXPECT_CALL(*this, GetPropertyPackageName())
      .WillOnce(Return(llvm::StringRef("")));

  EXPECT_CALL(*this, GetAdbClient(_))
      .WillOnce(Return(ByMove(AdbClientUP(adb_client))));

  Status result = DownloadModuleSlice(
      FileSpec("/system/app/Test/Test.apk!/lib/arm64-v8a/libtest.so"), 4096,
      3600, FileSpec("/tmp/libtest.so"));

  EXPECT_TRUE(result.Success());
}

TEST_F(PlatformAndroidTest,
       DownloadModuleSlice_ZipFileWithRunAs_UsesRunAsCommand) {
  auto *adb_client = new MockAdbClient();
  EXPECT_CALL(*adb_client,
              ShellToFile(StrEq("run-as 'com.example.test' "
                                "dd if='/system/app/Test/Test.apk' "
                                "iflag=skip_bytes,count_bytes "
                                "skip=4096 count=3600 status=none"),
                          _, _))
      .WillOnce(Return(Status()));

  EXPECT_CALL(*this, GetPropertyPackageName())
      .WillOnce(Return(llvm::StringRef("com.example.test")));

  EXPECT_CALL(*this, GetAdbClient(_))
      .WillOnce(Return(ByMove(AdbClientUP(adb_client))));

  Status result = DownloadModuleSlice(
      FileSpec("/system/app/Test/Test.apk!/lib/arm64-v8a/libtest.so"), 4096,
      3600, FileSpec("/tmp/libtest.so"));

  EXPECT_TRUE(result.Success());
}

TEST_F(PlatformAndroidTest,
       DownloadModuleSlice_LargeFile_CalculatesParametersCorrectly) {
  const uint64_t large_offset = 100 * 1024 * 1024; // 100MB offset
  const uint64_t large_size = 50 * 1024 * 1024;    // 50MB size

  auto *adb_client = new MockAdbClient();
  EXPECT_CALL(*adb_client,
              ShellToFile(StrEq("dd if='/system/app/Large.apk' "
                                "iflag=skip_bytes,count_bytes "
                                "skip=104857600 count=52428800 status=none"),
                          _, _))
      .WillOnce(Return(Status()));

  EXPECT_CALL(*this, GetPropertyPackageName())
      .WillOnce(Return(llvm::StringRef("")));

  EXPECT_CALL(*this, GetAdbClient(_))
      .WillOnce(Return(ByMove(AdbClientUP(adb_client))));

  Status result = DownloadModuleSlice(
      FileSpec("/system/app/Large.apk!/lib/arm64-v8a/large.so"), large_offset,
      large_size, FileSpec("/tmp/large.so"));

  EXPECT_TRUE(result.Success());
}

TEST_F(PlatformAndroidTest,
       GetFile_SyncServiceUnavailable_FallsBackToShellCat) {
  auto *adb_client = new MockAdbClient();
  EXPECT_CALL(*adb_client,
              ShellToFile(StrEq("cat '/data/local/tmp/test'"), _, _))
      .WillOnce(Return(Status()));

  EXPECT_CALL(*this, GetPropertyPackageName())
      .WillOnce(Return(llvm::StringRef("")));

  EXPECT_CALL(*this, GetAdbClient(_))
      .WillOnce(DoAll(WithArg<0>([](auto &arg) { arg.Clear(); }),
                      Return(ByMove(AdbClientUP(adb_client)))));

  EXPECT_CALL(*this, GetSyncService(_))
      .WillOnce([](Status &error) -> std::unique_ptr<AdbSyncService> {
        error = Status::FromErrorString("Sync service unavailable");
        return nullptr;
      });

  Status result =
      GetFile(FileSpec("/data/local/tmp/test"), FileSpec("/tmp/test"));
  EXPECT_TRUE(result.Success());
}

TEST_F(PlatformAndroidTest, GetFile_WithRunAs_UsesRunAsInShellCommand) {
  auto *adb_client = new MockAdbClient();
  EXPECT_CALL(
      *adb_client,
      ShellToFile(StrEq("run-as 'com.example.app' "
                        "cat '/data/data/com.example.app/lib-main/libtest.so'"),
                  _, _))
      .WillOnce(Return(Status()));

  EXPECT_CALL(*this, GetPropertyPackageName())
      .WillOnce(Return(llvm::StringRef("com.example.app")));

  EXPECT_CALL(*this, GetAdbClient(_))
      .WillOnce(DoAll(WithArg<0>([](auto &arg) { arg.Clear(); }),
                      Return(ByMove(AdbClientUP(adb_client)))));

  EXPECT_CALL(*this, GetSyncService(_))
      .WillOnce([](Status &error) -> std::unique_ptr<AdbSyncService> {
        error = Status::FromErrorString("Sync service unavailable");
        return nullptr;
      });

  Status result =
      GetFile(FileSpec("/data/data/com.example.app/lib-main/libtest.so"),
              FileSpec("/tmp/libtest.so"));
  EXPECT_TRUE(result.Success());
}

TEST_F(PlatformAndroidTest, GetFile_FilenameWithSingleQuotes_Rejected) {
  EXPECT_CALL(*this, GetSyncService(_))
      .WillOnce([](Status &error) -> std::unique_ptr<AdbSyncService> {
        error = Status::FromErrorString("Sync service unavailable");
        return nullptr;
      });

  Status result =
      GetFile(FileSpec("/test/file'with'quotes"), FileSpec("/tmp/output"));

  EXPECT_TRUE(result.Fail());
  EXPECT_THAT(result.AsCString(), HasSubstr("single-quotes"));
}

TEST_F(PlatformAndroidTest,
       DownloadModuleSlice_FilenameWithSingleQuotes_Rejected) {
  Status result = DownloadModuleSlice(FileSpec("/test/file'with'quotes"), 100,
                                      200, FileSpec("/tmp/output"));

  EXPECT_TRUE(result.Fail());
  EXPECT_THAT(result.AsCString(), HasSubstr("single-quotes"));
}

TEST_F(PlatformAndroidTest, GetFile_NetworkTimeout_PropagatesErrorCorrectly) {
  auto *adb_client = new MockAdbClient();
  EXPECT_CALL(*adb_client, ShellToFile(_, _, _))
      .WillOnce(Return(Status::FromErrorString("Network timeout")));

  EXPECT_CALL(*this, GetPropertyPackageName())
      .WillOnce(Return(llvm::StringRef("")));

  EXPECT_CALL(*this, GetAdbClient(_))
      .WillOnce(DoAll(WithArg<0>([](auto &arg) { arg.Clear(); }),
                      Return(ByMove(AdbClientUP(adb_client)))));

  EXPECT_CALL(*this, GetSyncService(_))
      .WillOnce([](Status &error) -> std::unique_ptr<AdbSyncService> {
        error = Status::FromErrorString("Sync service unavailable");
        return nullptr;
      });

  Status result =
      GetFile(FileSpec("/data/large/file.so"), FileSpec("/tmp/large.so"));
  EXPECT_TRUE(result.Fail());
  EXPECT_THAT(result.AsCString(), HasSubstr("Network timeout"));
}

TEST_F(PlatformAndroidTest, SyncService_ConnectionFailsGracefully) {
  // Constructor should succeed even with a failing connection
  AdbSyncService sync_service("test-device");

  // The service should report as not connected initially
  EXPECT_FALSE(sync_service.IsConnected());
  EXPECT_EQ(sync_service.GetDeviceId(), "test-device");

  // Operations should fail gracefully when connection setup fails
  FileSpec remote_file("/data/test.txt");
  FileSpec local_file("/tmp/test.txt");
  uint32_t mode, size, mtime;

  Status result = sync_service.Stat(remote_file, mode, size, mtime);
  EXPECT_TRUE(result.Fail());
}

TEST_F(PlatformAndroidTest, GetRunAs_FormatsPackageNameCorrectly) {
  // Empty package name
  EXPECT_CALL(*this, GetPropertyPackageName())
      .WillOnce(Return(llvm::StringRef("")));
  EXPECT_EQ(this->GetRunAs(), "");

  // Valid package name
  EXPECT_CALL(*this, GetPropertyPackageName())
      .WillOnce(Return(llvm::StringRef("com.example.test")));
  EXPECT_EQ(this->GetRunAs(), "run-as 'com.example.test' ");
}

TEST_F(PlatformAndroidTest,
       DownloadModuleSlice_ZeroOffset_CallsGetFileInsteadOfDd) {
  // When offset=0, DownloadModuleSlice calls GetFile which uses 'cat', not 'dd'
  // We need to ensure the sync service fails so GetFile falls back to shell cat
  auto *adb_client = new MockAdbClient();
  EXPECT_CALL(*adb_client,
              ShellToFile(StrEq("cat '/system/lib64/libc.so'"), _, _))
      .WillOnce(Return(Status()));

  EXPECT_CALL(*this, GetPropertyPackageName())
      .WillOnce(Return(llvm::StringRef("")));

  EXPECT_CALL(*this, GetAdbClient(_))
      .WillOnce(DoAll(WithArg<0>([](auto &arg) { arg.Clear(); }),
                      Return(ByMove(AdbClientUP(adb_client)))));

  // Mock GetSyncService to fail, forcing GetFile to use shell cat fallback
  EXPECT_CALL(*this, GetSyncService(_))
      .WillOnce(DoAll(WithArg<0>([](auto &arg) {
                        arg =
                            Status::FromErrorString("Sync service unavailable");
                      }),
                      Return(ByMove(std::unique_ptr<AdbSyncService>()))));

  Status result = DownloadModuleSlice(FileSpec("/system/lib64/libc.so"), 0, 0,
                                      FileSpec("/tmp/libc.so"));
  EXPECT_TRUE(result.Success());
}
