//===-- ClangUtilityFunction.cpp ------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ClangUtilityFunction.h"
#include "../../../../../clang/include/clang/AST/ASTContextAllocate.h"
#include "../../../../../clang/include/clang/AST/ASTFwd.h"
#include "../../../../../clang/include/clang/AST/ASTImportError.h"
#include "../../../../../clang/include/clang/AST/Attr.h"
#include "../../../../../clang/include/clang/AST/CanonicalType.h"
#include "../../../../../clang/include/clang/AST/CommentCommandTraits.h"
#include "../../../../../clang/include/clang/AST/ComparisonCategories.h"
#include "../../../../../clang/include/clang/AST/DeclBase.h"
#include "../../../../../clang/include/clang/AST/DeclCXX.h"
#include "../../../../../clang/include/clang/AST/DeclFriend.h"
#include "../../../../../clang/include/clang/AST/DeclObjC.h"
#include "../../../../../clang/include/clang/AST/Expr.h"
#include "../../../../../clang/include/clang/AST/OperationKinds.h"
#include "../../../../../clang/include/clang/AST/PrettyPrinter.h"
#include "../../../../../clang/include/clang/AST/RawCommentList.h"
#include "../../../../../clang/include/clang/AST/SYCLKernelInfo.h"
#include "../../../../../clang/include/clang/AST/Stmt.h"
#include "../../../../../clang/include/clang/AST/TypeBase.h"
#include "../../../../../clang/include/clang/AST/TypeLoc.h"
#include "../../../../../clang/include/clang/AST/TypeOrdering.h"
#include "../../../../../clang/include/clang/Analysis/Analyses/LifetimeSafety/LifetimeStats.h"
#include "../../../../../clang/include/clang/Basic/AttrKinds.h"
#include "../../../../../clang/include/clang/Basic/AttrSubjectMatchRules.h"
#include "../../../../../clang/include/clang/Basic/AttributeCommonInfo.h"
#include "../../../../../clang/include/clang/Basic/Builtins.h"
#include "../../../../../clang/include/clang/Basic/CFProtectionOptions.h"
#include "../../../../../clang/include/clang/Basic/CodeGenOptions.h"
#include "../../../../../clang/include/clang/Basic/CustomizableOptional.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticCategories.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticIDs.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticOptions.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticSema.h"
#include "../../../../../clang/include/clang/Basic/LangOptions.h"
#include "../../../../../clang/include/clang/Basic/LangStandard.h"
#include "../../../../../clang/include/clang/Basic/Module.h"
#include "../../../../../clang/include/clang/Basic/OffloadArch.h"
#include "../../../../../clang/include/clang/Basic/OpenMPKinds.h"
#include "../../../../../clang/include/clang/Basic/OperatorKinds.h"
#include "../../../../../clang/include/clang/Basic/OptionalUnsigned.h"
#include "../../../../../clang/include/clang/Basic/Sanitizers.h"
#include "../../../../../clang/include/clang/Basic/TargetCXXABI.h"
#include "../../../../../clang/include/clang/Basic/TargetInfo.h"
#include "../../../../../clang/include/clang/Basic/TokenKinds.h"
#include "../../../../../clang/include/clang/Lex/MacroBase.h"
#include "../../../../../clang/include/clang/Sema/DeclSpec.h"
#include "../../../../../clang/include/clang/Sema/ParsedAttr.h"
#include "../../../../../clang/include/clang/Sema/ScopeInfo.h"
#include "../../../../../clang/include/clang/Sema/Sema.h"
#include "../../../../../llvm/include/llvm/ADT/APFixedPoint.h"
#include "../../../../../llvm/include/llvm/ADT/TypeSwitch.h"
#include "../../../../../llvm/include/llvm/Frontend/HLSL/HLSLRootSignature.h"
#include "../../../../../llvm/include/llvm/Support/ConvertUTF.h"
#include "../../../../../llvm/include/llvm/Support/Registry.h"
#include "../../../../../llvm/include/llvm/TargetParser/AtomicScope.h"
#include "ClangExpressionDeclMap.h"
#include "ClangExpressionParser.h"
#include "ClangExpressionSourceCode.h"
#include "ClangPersistentVariables.h"

#include <cstdio>
#include <sys/types.h>


#include "lldb/Core/Module.h"
#include "lldb/Expression/IRExecutionUnit.h"

using namespace lldb_private;

char ClangUtilityFunction::ID;

ClangUtilityFunction::ClangUtilityFunction(ExecutionContextScope &exe_scope,
                                           std::string text, std::string name,
                                           bool enable_debugging)
    : UtilityFunction(
          exe_scope,
          std::string(ClangExpressionSourceCode::g_expression_prefix) + text +
              std::string(ClangExpressionSourceCode::g_expression_suffix),
          std::move(name), enable_debugging) {
  // Write the source code to a file so that LLDB's source manager can display
  // it when debugging the code.
  if (enable_debugging) {
    int temp_fd = -1;
    llvm::SmallString<128> result_path;
    llvm::sys::fs::createTemporaryFile("lldb", "expr", temp_fd, result_path);
    if (temp_fd != -1) {
      lldb_private::NativeFile file(temp_fd, File::eOpenOptionWriteOnly, true);
      text = "#line 1 \"" + std::string(result_path) + "\"\n" + text;
      size_t bytes_written = text.size();
      file.Write(text.c_str(), bytes_written);
      if (bytes_written == text.size()) {
        // If we successfully wrote the source to a temporary file, replace the
        // function text with the next text containing the line directive.
        m_function_text =
            std::string(ClangExpressionSourceCode::g_expression_prefix) + text +
            std::string(ClangExpressionSourceCode::g_expression_suffix);
      }
      file.Close();
    }
  }
}

ClangUtilityFunction::~ClangUtilityFunction() = default;

/// Install the utility function into a process
///
/// \param[in] diagnostic_manager
///     A diagnostic manager to report errors and warnings to.
///
/// \param[in] exe_ctx
///     The execution context to install the utility function to.
///
/// \return
///     True on success (no errors); false otherwise.
bool ClangUtilityFunction::Install(DiagnosticManager &diagnostic_manager,
                                   ExecutionContext &exe_ctx) {
  if (m_jit_start_addr != LLDB_INVALID_ADDRESS) {
    diagnostic_manager.PutString(lldb::eSeverityWarning, "already installed");
    return false;
  }

  ////////////////////////////////////
  // Set up the target and compiler
  //

  Target *target = exe_ctx.GetTargetPtr();

  if (!target) {
    diagnostic_manager.PutString(lldb::eSeverityError, "invalid target");
    return false;
  }

  Process *process = exe_ctx.GetProcessPtr();

  if (!process) {
    diagnostic_manager.PutString(lldb::eSeverityError, "invalid process");
    return false;
  }

  // Since we might need to call allocate memory and maybe call code to make
  // the caller, we need to be stopped.
  if (process->GetState() != lldb::eStateStopped) {
    diagnostic_manager.PutString(lldb::eSeverityError, "process running");
    return false;
  }
  //////////////////////////
  // Parse the expression
  //

  bool keep_result_in_memory = false;

  ResetDeclMap(exe_ctx, keep_result_in_memory);

  if (!DeclMap()->WillParse(exe_ctx, nullptr)) {
    diagnostic_manager.PutString(
        lldb::eSeverityError,
        "current process state is unsuitable for expression parsing");
    return false;
  }

  const bool generate_debug_info = true;
  ClangExpressionParser parser(exe_ctx.GetBestExecutionContextScope(), *this,
                               generate_debug_info, diagnostic_manager);

  unsigned num_errors = parser.Parse(diagnostic_manager);

  if (num_errors) {
    ResetDeclMap();

    return false;
  }

  //////////////////////////////////
  // JIT the output of the parser
  //

  bool can_interpret = false; // should stay that way

  Status jit_error = parser.PrepareForExecution(
      m_jit_start_addr, m_jit_end_addr, m_execution_unit_sp, exe_ctx,
      can_interpret, eExecutionPolicyAlways);

  if (m_jit_start_addr != LLDB_INVALID_ADDRESS) {
    m_jit_process_wp = process->shared_from_this();
    if (parser.GetGenerateDebugInfo()) {
      lldb::ModuleSP jit_module_sp(m_execution_unit_sp->GetJITModule());

      if (jit_module_sp) {
        ConstString const_func_name(FunctionName());
        FileSpec jit_file;
        jit_file.SetFilename(const_func_name);
        jit_module_sp->SetFileSpecAndObjectName(jit_file, ConstString());
        m_jit_module_wp = jit_module_sp;
        target->GetImages().Append(jit_module_sp);
      }
    }
  }

  DeclMap()->DidParse();

  ResetDeclMap();

  if (jit_error.Success()) {
    return true;
  } else {
    const char *error_cstr = jit_error.AsCString();
    if (error_cstr && error_cstr[0]) {
      diagnostic_manager.Printf(lldb::eSeverityError, "%s", error_cstr);
    } else {
      diagnostic_manager.PutString(lldb::eSeverityError,
                                   "expression can't be interpreted or run");
    }
    return false;
  }
}

char ClangUtilityFunction::ClangUtilityFunctionHelper::ID;

void ClangUtilityFunction::ClangUtilityFunctionHelper::ResetDeclMap(
    ExecutionContext &exe_ctx, bool keep_result_in_memory) {
  std::shared_ptr<ClangASTImporter> ast_importer;
  auto *state = exe_ctx.GetTargetSP()->GetPersistentExpressionStateForLanguage(
      lldb::eLanguageTypeC);
  if (state) {
    auto *persistent_vars = llvm::cast<ClangPersistentVariables>(state);
    ast_importer = persistent_vars->GetClangASTImporter();
  }
  m_expr_decl_map_up = std::make_unique<ClangExpressionDeclMap>(
      keep_result_in_memory, nullptr, exe_ctx.GetTargetSP(), ast_importer,
      nullptr, /*ignore_context_qualifiers=*/false);
}
