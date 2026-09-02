//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../../../clang/include/clang/Tooling/Core/Diagnostic.h"
#include "../../../clang/include/clang/Tooling/Core/Replacement.h"
#include "../../../llvm/include/llvm/Support/DynamicLibrary.h"
#include "../../../llvm/include/llvm/Support/Errc.h"
#include "../../../llvm/include/llvm/Support/ExtensibleRTTI.h"
#include "../../../llvm/include/llvm/Support/FileSystem.h"
#include "../../../llvm/include/llvm/Support/Path.h"
#include "../../../llvm/include/llvm/Support/Registry.h"
#include "../../../llvm/include/llvm/Support/SMLoc.h"
#include "../../../llvm/include/llvm/Support/SourceMgr.h"
#include "../../../llvm/include/llvm/Support/VirtualFileSystem.h"
#include "../ClangTidy.h"
#include "../ClangTidyDiagnosticConsumer.h"
#include "../ClangTidyModule.h"
#include "../ClangTidyOptions.h"
#include "../ClangTidyProfiling.h"
#include "../FileExtensionsSet.h"
#include "../NoLintDirectiveHandler.h"
#include "AssertEqualsCheck.h"
#include "AvoidNSErrorInitCheck.h"
#include "DeallocInCategoryCheck.h"
#include "ForbiddenSubclassingCheck.h"
#include "MissingHashCheck.h"
#include "NSDateFormatterCheck.h"
#include "NSInvocationArgumentLifetimeCheck.h"
#include "PropertyDeclarationCheck.h"
#include "SuperSelfCheck.h"

using namespace clang::ast_matchers;

namespace clang::tidy {
namespace objc {
namespace {

class ObjCModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<AvoidNSErrorInitCheck>(
        "objc-avoid-nserror-init");
    CheckFactories.registerCheck<AssertEqualsCheck>("objc-assert-equals");

    CheckFactories.registerCheck<DeallocInCategoryCheck>(
        "objc-dealloc-in-category");
    CheckFactories.registerCheck<ForbiddenSubclassingCheck>(
        "objc-forbidden-subclassing");
    CheckFactories.registerCheck<MissingHashCheck>("objc-missing-hash");
    CheckFactories.registerCheck<NSDateFormatterCheck>("objc-nsdate-formatter");
    CheckFactories.registerCheck<NSInvocationArgumentLifetimeCheck>(
        "objc-nsinvocation-argument-lifetime");
    CheckFactories.registerCheck<PropertyDeclarationCheck>(
        "objc-property-declaration");
    CheckFactories.registerCheck<SuperSelfCheck>("objc-super-self");
  }
};

} // namespace

// Register the ObjCTidyModule using this statically initialized variable.
static ClangTidyModuleRegistry::Add<ObjCModule>
    X("objc-module", "Adds Objective-C lint checks.");

} // namespace objc

// This anchor is used to force the linker to link in the generated object file
// and thus register the ObjCModule.
volatile int ObjCModuleAnchorSource = 0; // NOLINT(misc-use-internal-linkage)

} // namespace clang::tidy
