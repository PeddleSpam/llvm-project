//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../../../clang/include/clang/ASTMatchers/ASTMatchFinder.h"
#include "../../../clang/include/clang/Tooling/Core/Diagnostic.h"
#include "../../../clang/include/clang/Tooling/Core/Replacement.h"
#include "../../../llvm/include/llvm/Support/Registry.h"
#include "../../../llvm/include/llvm/Support/Timer.h"
#include "../../../llvm/include/llvm/Support/VirtualFileSystem.h"
#include "../ClangTidy.h"
#include "../ClangTidyCheck.h"
#include "../ClangTidyDiagnosticConsumer.h"
#include "../ClangTidyModule.h"
#include "../ClangTidyOptions.h"
#include "../ClangTidyProfiling.h"
#include "../FileExtensionsSet.h"
#include "../NoLintDirectiveHandler.h"
#include "../utils/IncludeSorter.h"
#include "AvoidEndlCheck.h"
#include "EnumSizeCheck.h"
#include "ExpensiveValueOrCheck.h"
#include "ForRangeCopyCheck.h"
#include "ImplicitConversionInLoopCheck.h"
#include "InefficientAlgorithmCheck.h"
#include "InefficientStringConcatenationCheck.h"
#include "InefficientVectorOperationCheck.h"
#include "MoveConstArgCheck.h"
#include "MoveConstructorInitCheck.h"
#include "NoAutomaticMoveCheck.h"
#include "NoIntToPtrCheck.h"
#include "NoexceptDestructorCheck.h"
#include "NoexceptMoveConstructorCheck.h"
#include "NoexceptSwapCheck.h"
#include "PreferSingleCharOverloadsCheck.h"
#include "StringViewConversionsCheck.h"
#include "TriviallyDestructibleCheck.h"
#include "TypePromotionInMathFnCheck.h"
#include "UnnecessaryCopyInitializationCheck.h"
#include "UnnecessaryValueParamCheck.h"
#include "UseStdMoveCheck.h"

namespace clang::tidy {
namespace performance {
namespace {

class PerformanceModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<AvoidEndlCheck>("performance-avoid-endl");
    CheckFactories.registerCheck<EnumSizeCheck>("performance-enum-size");
    CheckFactories.registerCheck<ExpensiveValueOrCheck>(
        "performance-expensive-value-or");
    CheckFactories.registerCheck<PreferSingleCharOverloadsCheck>(
        "performance-faster-string-find");
    CheckFactories.registerCheck<ForRangeCopyCheck>(
        "performance-for-range-copy");
    CheckFactories.registerCheck<ImplicitConversionInLoopCheck>(
        "performance-implicit-conversion-in-loop");
    CheckFactories.registerCheck<InefficientAlgorithmCheck>(
        "performance-inefficient-algorithm");
    CheckFactories.registerCheck<InefficientStringConcatenationCheck>(
        "performance-inefficient-string-concatenation");
    CheckFactories.registerCheck<InefficientVectorOperationCheck>(
        "performance-inefficient-vector-operation");
    CheckFactories.registerCheck<MoveConstArgCheck>(
        "performance-move-const-arg");
    CheckFactories.registerCheck<MoveConstructorInitCheck>(
        "performance-move-constructor-init");
    CheckFactories.registerCheck<NoAutomaticMoveCheck>(
        "performance-no-automatic-move");
    CheckFactories.registerCheck<NoIntToPtrCheck>("performance-no-int-to-ptr");
    CheckFactories.registerCheck<NoexceptDestructorCheck>(
        "performance-noexcept-destructor");
    CheckFactories.registerCheck<NoexceptMoveConstructorCheck>(
        "performance-noexcept-move-constructor");
    CheckFactories.registerCheck<NoexceptSwapCheck>(
        "performance-noexcept-swap");
    CheckFactories.registerCheck<PreferSingleCharOverloadsCheck>(
        "performance-prefer-single-char-overloads");
    CheckFactories.registerCheck<StringViewConversionsCheck>(
        "performance-string-view-conversions");
    CheckFactories.registerCheck<TriviallyDestructibleCheck>(
        "performance-trivially-destructible");
    CheckFactories.registerCheck<TypePromotionInMathFnCheck>(
        "performance-type-promotion-in-math-fn");
    CheckFactories.registerCheck<UnnecessaryCopyInitializationCheck>(
        "performance-unnecessary-copy-initialization");
    CheckFactories.registerCheck<UnnecessaryValueParamCheck>(
        "performance-unnecessary-value-param");
    CheckFactories.registerCheck<UseStdMoveCheck>("performance-use-std-move");
  }
};

} // namespace

// Register the PerformanceModule using this statically initialized variable.
static ClangTidyModuleRegistry::Add<PerformanceModule>
    X("performance-module", "Adds performance checks.");

} // namespace performance

// This anchor is used to force the linker to link in the generated object file
// and thus register the PerformanceModule.
// NOLINTNEXTLINE(misc-use-internal-linkage)
volatile int PerformanceModuleAnchorSource = 0;

} // namespace clang::tidy
