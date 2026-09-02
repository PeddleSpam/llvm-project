#include "../../../clang/include/clang/ASTMatchers/ASTMatchFinder.h"
#include "../../../clang/include/clang/Tooling/Core/Diagnostic.h"
#include "../../../clang/include/clang/Tooling/Core/Replacement.h"
#include "../../../llvm/include/llvm/Support/DynamicLibrary.h"
#include "../../../llvm/include/llvm/Support/Errc.h"
#include "../../../llvm/include/llvm/Support/ExtensibleRTTI.h"
#include "../../../llvm/include/llvm/Support/FileSystem.h"
#include "../../../llvm/include/llvm/Support/Path.h"
#include "../../../llvm/include/llvm/Support/Registry.h"
#include "../../../llvm/include/llvm/Support/SourceMgr.h"
#include "../../../llvm/include/llvm/Support/Timer.h"
#include "../../../llvm/include/llvm/Support/VirtualFileSystem.h"
#include "../ClangTidy.h"
#include "../ClangTidyDiagnosticConsumer.h"
#include "../ClangTidyModule.h"
#include "../ClangTidyOptions.h"
#include "../ClangTidyProfiling.h"
#include "../FileExtensionsSet.h"
#include "../NoLintDirectiveHandler.h"
#include "QueryCheck.h"
#include <cassert>
#include <memory>

namespace clang::tidy {
namespace custom {

// We need to register the checks more flexibly than builtin modules. The checks
// will changed dynamically when switching to different source file.
static void registerCustomChecks(const ClangTidyOptions &Options,
                                 ClangTidyCheckFactories &Factories) {
  static llvm::SmallSet<SmallString<32>, 8> CustomCheckNames{};
  if (!Options.CustomChecks.has_value() || Options.CustomChecks->empty())
    return;
  for (const SmallString<32> &Name : CustomCheckNames)
    Factories.eraseCheck(Name);
  for (const ClangTidyOptions::CustomCheckValue &V :
       Options.CustomChecks.value()) {
    SmallString<32> Name = StringRef{"custom-" + V.Name};
    Factories.registerCheckFactory(
        // add custom- prefix to avoid conflicts with builtin checks
        Name, [&V](StringRef Name, ClangTidyContext *Context) {
          return std::make_unique<custom::QueryCheck>(Name, V, Context);
        });
    CustomCheckNames.insert(std::move(Name));
  }
}

namespace {

struct CustomChecksRegisterInitializer {
  CustomChecksRegisterInitializer() noexcept {
    RegisterCustomChecks = &custom::registerCustomChecks;
  }
};

class CustomModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {}
};

} // namespace

static CustomChecksRegisterInitializer Init{};

} // namespace custom

// Register the CustomTidyModule using this statically initialized variable.
static ClangTidyModuleRegistry::Add<custom::CustomModule>
    X("custom-module", "Adds custom query lint checks.");

// This anchor is used to force the linker to link in the generated object file
// and thus register the AlteraModule.
volatile int CustomModuleAnchorSource = 0; // NOLINT (misc-use-internal-linkage)

} // namespace clang::tidy
