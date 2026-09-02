//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../../../clang/include/clang/Format/Format.h"
#include "../../../clang/include/clang/Tooling/Core/Diagnostic.h"
#include "../../../clang/include/clang/Tooling/Core/Replacement.h"
#include "../../../clang/include/clang/Tooling/Refactoring/AtomicChange.h"
#include "../../../clang/include/clang/Tooling/Transformer/MatchConsumer.h"
#include "../../../clang/include/clang/Tooling/Transformer/RangeSelector.h"
#include "../../../clang/include/clang/Tooling/Transformer/RewriteRule.h"
#include "../../../clang/include/clang/Tooling/Transformer/Transformer.h"
#include "../../../llvm/include/llvm/ADT/Any.h"
#include "../../../llvm/include/llvm/Support/Chrono.h"
#include "../../../llvm/include/llvm/Support/DynamicLibrary.h"
#include "../../../llvm/include/llvm/Support/ExtensibleRTTI.h"
#include "../../../llvm/include/llvm/Support/FileSystem.h"
#include "../../../llvm/include/llvm/Support/Path.h"
#include "../../../llvm/include/llvm/Support/Registry.h"
#include "../../../llvm/include/llvm/Support/VirtualFileSystem.h"
#include "../ClangTidy.h"
#include "../ClangTidyDiagnosticConsumer.h"
#include "../ClangTidyModule.h"
#include "../ClangTidyOptions.h"
#include "../ClangTidyProfiling.h"
#include "../FileExtensionsSet.h"
#include "../NoLintDirectiveHandler.h"
#include "../utils/IncludeSorter.h"
#include "CleanupCtadCheck.h"
#include "DurationAdditionCheck.h"
#include "DurationComparisonCheck.h"
#include "DurationConversionCastCheck.h"
#include "DurationDivisionCheck.h"
#include "DurationFactoryFloatCheck.h"
#include "DurationFactoryScaleCheck.h"
#include "DurationSubtractionCheck.h"
#include "DurationUnnecessaryConversionCheck.h"
#include "FasterStrsplitDelimiterCheck.h"
#include "NoInternalDependenciesCheck.h"
#include "NoNamespaceCheck.h"
#include "RedundantStrcatCallsCheck.h"
#include "StrCatAppendCheck.h"
#include "StringFindStartswithCheck.h"
#include "StringFindStrContainsCheck.h"
#include "TimeComparisonCheck.h"
#include "TimeSubtractionCheck.h"
#include "UncheckedStatusOrAccessCheck.h"
#include "UpgradeDurationConversionsCheck.h"

namespace clang::tidy {
namespace abseil {
namespace {

class AbseilModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<CleanupCtadCheck>("abseil-cleanup-ctad");
    CheckFactories.registerCheck<DurationAdditionCheck>(
        "abseil-duration-addition");
    CheckFactories.registerCheck<DurationComparisonCheck>(
        "abseil-duration-comparison");
    CheckFactories.registerCheck<DurationConversionCastCheck>(
        "abseil-duration-conversion-cast");
    CheckFactories.registerCheck<DurationDivisionCheck>(
        "abseil-duration-division");
    CheckFactories.registerCheck<DurationFactoryFloatCheck>(
        "abseil-duration-factory-float");
    CheckFactories.registerCheck<DurationFactoryScaleCheck>(
        "abseil-duration-factory-scale");
    CheckFactories.registerCheck<DurationSubtractionCheck>(
        "abseil-duration-subtraction");
    CheckFactories.registerCheck<DurationUnnecessaryConversionCheck>(
        "abseil-duration-unnecessary-conversion");
    CheckFactories.registerCheck<FasterStrsplitDelimiterCheck>(
        "abseil-faster-strsplit-delimiter");
    CheckFactories.registerCheck<NoInternalDependenciesCheck>(
        "abseil-no-internal-dependencies");
    CheckFactories.registerCheck<NoNamespaceCheck>("abseil-no-namespace");
    CheckFactories.registerCheck<RedundantStrcatCallsCheck>(
        "abseil-redundant-strcat-calls");
    CheckFactories.registerCheck<StrCatAppendCheck>("abseil-str-cat-append");
    CheckFactories.registerCheck<StringFindStartswithCheck>(
        "abseil-string-find-startswith");
    CheckFactories.registerCheck<StringFindStrContainsCheck>(
        "abseil-string-find-str-contains");
    CheckFactories.registerCheck<TimeComparisonCheck>("abseil-time-comparison");
    CheckFactories.registerCheck<TimeSubtractionCheck>(
        "abseil-time-subtraction");
    CheckFactories.registerCheck<UncheckedStatusOrAccessCheck>(
        "abseil-unchecked-statusor-access");
    CheckFactories.registerCheck<UpgradeDurationConversionsCheck>(
        "abseil-upgrade-duration-conversions");
  }
};

} // namespace

// Register the AbseilModule using this statically initialized variable.
static ClangTidyModuleRegistry::Add<AbseilModule> X("abseil-module",
                                                    "Add Abseil checks.");

} // namespace abseil

// This anchor is used to force the linker to link in the generated object file
// and thus register the AbseilModule.
volatile int AbseilModuleAnchorSource = 0; // NOLINT(misc-use-internal-linkage)

} // namespace clang::tidy
