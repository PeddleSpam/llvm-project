//===-- ClangExpressionDeclMapTest.cpp ------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Plugins/ExpressionParser/Clang/ClangExpressionDeclMap.h"
#include "../../../clang/include/clang/AST/ASTContextAllocate.h"
#include "../../../clang/include/clang/AST/ASTFwd.h"
#include "../../../clang/include/clang/AST/ASTImportError.h"
#include "../../../clang/include/clang/AST/Attr.h"
#include "../../../clang/include/clang/AST/CanonicalType.h"
#include "../../../clang/include/clang/AST/CommentCommandTraits.h"
#include "../../../clang/include/clang/AST/ComparisonCategories.h"
#include "../../../clang/include/clang/AST/DeclBase.h"
#include "../../../clang/include/clang/AST/DeclCXX.h"
#include "../../../clang/include/clang/AST/DeclFriend.h"
#include "../../../clang/include/clang/AST/DeclObjC.h"
#include "../../../clang/include/clang/AST/Expr.h"
#include "../../../clang/include/clang/AST/OperationKinds.h"
#include "../../../clang/include/clang/AST/PrettyPrinter.h"
#include "../../../clang/include/clang/AST/RawCommentList.h"
#include "../../../clang/include/clang/AST/SYCLKernelInfo.h"
#include "../../../clang/include/clang/AST/Stmt.h"
#include "../../../clang/include/clang/AST/TypeBase.h"
#include "../../../clang/include/clang/AST/TypeLoc.h"
#include "../../../clang/include/clang/AST/TypeOrdering.h"
#include "../../../clang/include/clang/Analysis/Analyses/LifetimeSafety/LifetimeStats.h"
#include "../../../clang/include/clang/Basic/AttrKinds.h"
#include "../../../clang/include/clang/Basic/AttrSubjectMatchRules.h"
#include "../../../clang/include/clang/Basic/AttributeCommonInfo.h"
#include "../../../clang/include/clang/Basic/Builtins.h"
#include "../../../clang/include/clang/Basic/CFProtectionOptions.h"
#include "../../../clang/include/clang/Basic/CodeGenOptions.h"
#include "../../../clang/include/clang/Basic/CustomizableOptional.h"
#include "../../../clang/include/clang/Basic/DiagnosticCategories.h"
#include "../../../clang/include/clang/Basic/DiagnosticIDs.h"
#include "../../../clang/include/clang/Basic/DiagnosticOptions.h"
#include "../../../clang/include/clang/Basic/DiagnosticSema.h"
#include "../../../clang/include/clang/Basic/DirectoryEntry.h"
#include "../../../clang/include/clang/Basic/FileEntry.h"
#include "../../../clang/include/clang/Basic/LangOptions.h"
#include "../../../clang/include/clang/Basic/LangStandard.h"
#include "../../../clang/include/clang/Basic/OffloadArch.h"
#include "../../../clang/include/clang/Basic/OpenMPKinds.h"
#include "../../../clang/include/clang/Basic/OperatorKinds.h"
#include "../../../clang/include/clang/Basic/OptionalUnsigned.h"
#include "../../../clang/include/clang/Basic/Sanitizers.h"
#include "../../../clang/include/clang/Basic/TargetCXXABI.h"
#include "../../../clang/include/clang/Basic/TargetInfo.h"
#include "../../../clang/include/clang/Basic/TokenKinds.h"
#include "../../../clang/include/clang/Lex/MacroBase.h"
#include "../../../clang/include/clang/Sema/DeclSpec.h"
#include "../../../clang/include/clang/Sema/ParsedAttr.h"
#include "../../../clang/include/clang/Sema/ScopeInfo.h"
#include "../../../clang/include/clang/Sema/Sema.h"
#include "../../../llvm/include/llvm/ADT/APFixedPoint.h"
#include "../../../llvm/include/llvm/ADT/FunctionExtras.h"
#include "../../../llvm/include/llvm/ADT/TypeSwitch.h"
#include "../../../llvm/include/llvm/Frontend/HLSL/HLSLRootSignature.h"
#include "../../../llvm/include/llvm/Support/ConvertUTF.h"
#include "../../../llvm/include/llvm/Support/Registry.h"
#include "../../../llvm/include/llvm/Support/TypeSize.h"
#include "../../../llvm/include/llvm/TargetParser/AtomicScope.h"
#include "TestingSupport/SubsystemRAII.h"
#include "TestingSupport/Symbol/ClangTestUtils.h"

using namespace lldb_private;
using namespace lldb;

namespace {
struct FakeClangExpressionDeclMap : public ClangExpressionDeclMap {
  FakeClangExpressionDeclMap(const std::shared_ptr<ClangASTImporter> &importer)
      : ClangExpressionDeclMap(false, nullptr, lldb::TargetSP(), importer,
                               nullptr, /*ignore_context_qualifiers=*/false) {
    m_holder = std::make_unique<clang_utils::TypeSystemClangHolder>("ast");
    m_scratch_context = m_holder->GetAST();
  }
  std::unique_ptr<clang_utils::TypeSystemClangHolder> m_holder;
  TypeSystemClang *m_scratch_context;
  /// Adds a persistent decl that can be found by the ClangExpressionDeclMap
  /// via GetPersistentDecl.
  void AddPersistentDeclForTest(clang::NamedDecl *d) {
    // The declaration needs to have '$' prefix in its name like every
    // persistent declaration and must be inside the scratch AST context.
    assert(d);
    assert(d->getName().starts_with("$"));
    assert(&d->getASTContext() == &m_scratch_context->getASTContext());
    m_persistent_decls[d->getName()] = d;
  }

protected:
  // ClangExpressionDeclMap hooks.

  clang::NamedDecl *GetPersistentDecl(ConstString name) override {
    // ClangExpressionDeclMap wants to know if there is a persistent decl
    // with the given name. Check the
    return m_persistent_decls.lookup(name.GetStringRef());
  }

private:
  /// The persistent decls in this test with their names as keys.
  llvm::DenseMap<llvm::StringRef, clang::NamedDecl *> m_persistent_decls;
};
} // namespace

namespace {
struct ClangExpressionDeclMapTest : public testing::Test {
  SubsystemRAII<FileSystem, HostInfo> subsystems;

  /// The ClangASTImporter used during the test.
  std::shared_ptr<ClangASTImporter> importer;
  /// The ExpressionDeclMap for the current test case.
  std::unique_ptr<FakeClangExpressionDeclMap> decl_map;

  std::unique_ptr<clang_utils::TypeSystemClangHolder> holder;
  
  /// The target AST that lookup results should be imported to.
  TypeSystemClang *target_ast;

  void SetUp() override {
    importer = std::make_shared<ClangASTImporter>();
    decl_map = std::make_unique<FakeClangExpressionDeclMap>(importer);
    holder = std::make_unique<clang_utils::TypeSystemClangHolder>("target ast");
    target_ast = holder->GetAST();
    decl_map->InstallASTContext(*target_ast);
  }

  void TearDown() override {
    importer.reset();
    decl_map.reset();
    holder.reset();
  }
};
} // namespace

TEST_F(ClangExpressionDeclMapTest, TestUnknownIdentifierLookup) {
  // Tests looking up an identifier that can't be found anywhere.

  // Setup a NameSearchContext for 'foo'.
  llvm::SmallVector<clang::NamedDecl *, 16> decls;
  clang::DeclarationName name =
      clang_utils::getDeclarationName(*target_ast, "foo");
  const clang::DeclContext *dc = target_ast->GetTranslationUnitDecl();
  NameSearchContext search(*target_ast, decls, name, dc);

  decl_map->FindExternalVisibleDecls(search);

  // This shouldn't exist so we should get no lookups.
  EXPECT_EQ(0U, decls.size());
}

TEST_F(ClangExpressionDeclMapTest, TestPersistentDeclLookup) {
  // Tests looking up a persistent decl from the scratch AST context.

  // Create a '$persistent_class' record and add it as a persistent variable
  // to the scratch AST context.
  llvm::StringRef decl_name = "$persistent_class";
  CompilerType persistent_type =
      clang_utils::createRecord(*decl_map->m_scratch_context, decl_name);
  decl_map->AddPersistentDeclForTest(ClangUtil::GetAsTagDecl(persistent_type));

  // Setup a NameSearchContext for $persistent_class;
  llvm::SmallVector<clang::NamedDecl *, 16> decls;
  clang::DeclarationName name =
      clang_utils::getDeclarationName(*target_ast, decl_name);
  const clang::DeclContext *dc = target_ast->GetTranslationUnitDecl();
  NameSearchContext search(*target_ast, decls, name, dc);

  // Search and check that we found $persistent_class.
  decl_map->FindExternalVisibleDecls(search);
  EXPECT_EQ(1U, decls.size());
  EXPECT_EQ(decl_name, decls.front()->getQualifiedNameAsString());
  auto *record = llvm::cast<clang::RecordDecl>(decls.front());
  // The class was minimally imported from the scratch AST context.
  EXPECT_TRUE(record->hasExternalLexicalStorage());
}
