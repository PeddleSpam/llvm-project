//===-- TweakTests.cpp ------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../../../../llvm/include/llvm/ADT/DenseMapInfoVariant.h"
#include "../../../include-cleaner/include/clang-include-cleaner/Types.h"
#include "TestFS.h"
#include "refactor/Tweak.h"
#include "llvm/Testing/Support/Error.h"
#include <cassert>
#include <string>
#include <utility>
#include <vector>

namespace clang {
namespace clangd {
namespace {

TEST(FileEdits, AbsolutePath) {
  auto RelPaths = {"a.h", "foo.cpp", "test/test.cpp"};

  llvm::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> MemFS(
      new llvm::vfs::InMemoryFileSystem);
  MemFS->setCurrentWorkingDirectory(testRoot());
  for (const auto *Path : RelPaths)
    MemFS->addFile(Path, 0, llvm::MemoryBuffer::getMemBuffer("", Path));
  FileManager FM(FileSystemOptions(), MemFS);
  DiagnosticOptions DiagOpts;
  DiagnosticsEngine DE(DiagnosticIDs::create(), DiagOpts);
  SourceManager SM(DE, FM);

  for (const auto *Path : RelPaths) {
    auto FID = SM.createFileID(*FM.getOptionalFileRef(Path), SourceLocation(),
                               clang::SrcMgr::C_User);
    auto Res = Tweak::Effect::fileEdit(SM, FID, tooling::Replacements());
    ASSERT_THAT_EXPECTED(Res, llvm::Succeeded());
    EXPECT_EQ(Res->first, testPath(Path));
  }
}

} // namespace
} // namespace clangd
} // namespace clang
