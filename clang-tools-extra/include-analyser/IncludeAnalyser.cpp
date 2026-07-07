//===--- IncludeAnalyser.cpp - Standalone tool for include optimisation. --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include "IAActionFactory.h"
#include "IAOptions.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/Signals.h"
#include <fstream>

namespace clang {
namespace include_analyser {
namespace {
namespace cl = llvm::cl;

auto overview = llvm::StringLiteral(R"(
include-analyser optimises the #include directives in source code.
It suggests including only the headers that the code uses.
What it considers to be a *use* is more restrictive than clang-include-cleaner.
)")
                    .trim();

auto includeAnalyser = cl::OptionCategory("include-analyser");

auto reportPath = cl::opt<std::string>{
    "html",
    cl::desc("Specify an output filename for an HTML report. "
             "This describes both recommendations and reasons for changes."),
    cl::cat(includeAnalyser),
};

auto onlyHeaders = cl::opt<std::string>{
    "only-headers",
    cl::desc("A comma-separated list of regular expressions to match against "
             "the path of a header. Only headers that match will be analyzed."),
    cl::init(""),
    cl::cat(includeAnalyser),
};

auto ignoreHeaders = cl::opt<std::string>{
    "ignore-headers",
    cl::desc("A comma-separated list of regular expressions to match against "
             "the path of a header, and disable analysis if matched."),
    cl::init(""),
    cl::cat(includeAnalyser),
};

auto edit = cl::opt<bool>{
    "edit",
    cl::desc("Apply edits to analyzed source files."),
    cl::cat(includeAnalyser),
};

auto insert = cl::opt<bool>{
    "insert",
    cl::desc(
        "Allow header insertions (Deprecated. Use -disable-insert instead)."),
    cl::init(true),
    cl::cat(includeAnalyser),
};

auto remove = cl::opt<bool>{
    "remove",
    cl::desc(
        "Allow header removals (Deprecated. Use -disable-remove instead)."),
    cl::init(true),
    cl::cat(includeAnalyser),
};

auto disableInsert = cl::opt<bool>{
    "disable-insert",
    cl::desc("Disable header insertions."),
    cl::init(false),
    cl::cat(includeAnalyser),
};

auto disableRemove = cl::opt<bool>{
    "disable-remove",
    cl::desc("Disable header removals."),
    cl::init(false),
    cl::cat(includeAnalyser),
};

using PrintStyle = IAOptions::PrintStyle;

auto print = cl::opt<PrintStyle>{
    "print",
    cl::values(
        clEnumValN(PrintStyle::Changes, "changes", "Print symbolic changes"),
        clEnumValN(PrintStyle::Final, "", "Print final code")),
    cl::ValueOptional,
    cl::init(PrintStyle::Final),
    cl::desc("Print the list of headers to insert and remove."),
    cl::cat(includeAnalyser),
};

std::function<bool(llvm::StringRef)> getMatcher(llvm::StringRef regexFlag) {
  auto filterRegs = std::make_shared<std::vector<llvm::Regex>>();
  auto headers = llvm::SmallVector<llvm::StringRef>();
  regexFlag.split(headers, ',', -1, false);
  for (auto pattern : headers) {
    auto anchoredPattern = std::string("(" + pattern.str() + ")$");
    auto compiledRegex = llvm::Regex(anchoredPattern);
    auto regexError = std::string();
    if (!compiledRegex.isValid(regexError)) {
      llvm::errs() << llvm::formatv("Invalid regular expression '{0}': {1}\n",
                                    pattern, regexError);
      return nullptr;
    }
    filterRegs->push_back(std::move(compiledRegex));
  }
  return [filterRegs](llvm::StringRef Path) {
    for (auto const &f : *filterRegs) {
      if (f.match(Path))
        return true;
    }
    return false;
  };
}

std::function<bool(llvm::StringRef)> headerFilter() {
  auto onlyMatches = getMatcher(onlyHeaders);
  auto ignoreMatches = getMatcher(ignoreHeaders);
  if (!onlyMatches || !ignoreMatches)
    return nullptr;

  return [onlyMatches, ignoreMatches](llvm::StringRef header) {
    if (!onlyHeaders.empty() && !onlyMatches(header))
      return true;
    if (!ignoreHeaders.empty() && ignoreMatches(header))
      return true;
    return false;
  };
}

llvm::Expected<std::map<std::string, std::string>>
mapInputsToAbsPaths(clang::tooling::CompilationDatabase &compDB,
                    llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> fileSys,
                    std::vector<std::string> const &inputs) {
  auto compDBToAbsPaths = std::map<std::string, std::string>();
  for (auto &source : inputs) {
    auto absPath = llvm::SmallString<256u>(source);
    if (auto err = fileSys->makeAbsolute(absPath)) {
      llvm::errs() << "Failed to get absolute path for " << source << " : "
                   << err.message() << '\n';
      return llvm::errorCodeToError(err);
    }

    auto commands = std::vector<clang::tooling::CompileCommand>(
        compDB.getCompileCommands(absPath));
    if (commands.empty()) {
      auto errorMsg =
          llvm::formatv("No compile commands found for {0}", absPath).str();
      llvm::errs() << errorMsg << '\n';
      return llvm::make_error<llvm::StringError>(
          errorMsg, llvm::inconvertibleErrorCode());
    }

    for (auto const &cmd : commands) {
      auto path = llvm::SmallString<256>(cmd.Filename);
      llvm::sys::path::make_absolute(cmd.Directory, path);
      compDBToAbsPaths[std::string(path)] = std::string(absPath);
    }
  }
  return compDBToAbsPaths;
}

} // namespace
} // namespace include_analyser
} // namespace clang

int main(int argc, char const** argv) {
  using namespace clang::include_analyser;

  llvm::sys::PrintStackTraceOnErrorSignal(argv[0]);
  auto optionsParser =
      clang::tooling::CommonOptionsParser::create(argc, argv, includeAnalyser);
  if (!optionsParser) {
    llvm::errs() << toString(optionsParser.takeError());
    return 1;
  }

  if (optionsParser->getSourcePathList().size() != 1) {
    auto incompatibleFlags = std::vector<cl::Option*>{&reportPath, &print};
    for (auto const* flag : incompatibleFlags) {
      if (flag->getNumOccurrences()) {
        llvm::errs() << "-" << flag->ArgStr << " requires a single input file";
        return 1;
      }
    }
  }

  auto fileSys = llvm::vfs::getRealFileSystem();
  auto& compDB = optionsParser->getCompilations();
  auto compDBToAbsPaths =
      mapInputsToAbsPaths(compDB, fileSys, optionsParser->getSourcePathList());
  if (!compDBToAbsPaths)
    return 1;

  auto tool =
      clang::tooling::ClangTool(compDB, optionsParser->getSourcePathList());
  auto filter = clang::include_analyser::headerFilter();
  if (!filter)
    return 1;

  auto options = IAOptions(reportPath.getValue(), filter, edit.getValue(),
                           disableInsert.getValue(), disableRemove.getValue(), 
                           print.getValue());
  auto factory = IAActionFactory(options);
  auto errCode = tool.run(&factory);

  // Write results to files.
  auto errorCount = 0u;
  if (edit.getValue()) {
    for (auto &result : factory.getEditedFiles()) {
      auto fileName = result.first();
      auto iter = compDBToAbsPaths->find(fileName.str());
      if (iter != compDBToAbsPaths->end()) {
        fileName = iter->second;
      }
      auto err = llvm::writeToOutput(
          fileName.str(), [&](llvm::raw_ostream &strm) -> llvm::Error {
            strm << result.second;
            return llvm::Error::success();
          });
      if (err) {
        llvm::errs() << "Failed to apply edits to " << fileName << ": "
                     << llvm::toString(std::move(err)) << "\n";
        ++errorCount;
      }
    }
  }

  return errCode || errorCount != 0u;
}
