//===-- ClangFunctionCaller.cpp -------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//


#include "../../../../../clang/include/clang/APINotes/APINotesOptions.h"
#include "../../../../../clang/include/clang/AST/ASTConsumer.h"
#include "../../../../../clang/include/clang/AST/ASTContextAllocate.h"
#include "../../../../../clang/include/clang/AST/ASTFwd.h"
#include "../../../../../clang/include/clang/AST/Attr.h"
#include "../../../../../clang/include/clang/AST/CanonicalType.h"
#include "../../../../../clang/include/clang/AST/CommentCommandTraits.h"
#include "../../../../../clang/include/clang/AST/ComparisonCategories.h"
#include "../../../../../clang/include/clang/AST/DeclBase.h"
#include "../../../../../clang/include/clang/AST/DeclCXX.h"
#include "../../../../../clang/include/clang/AST/DeclID.h"
#include "../../../../../clang/include/clang/AST/Expr.h"
#include "../../../../../clang/include/clang/AST/ExternalASTSource.h"
#include "../../../../../clang/include/clang/AST/NestedNameSpecifierBase.h"
#include "../../../../../clang/include/clang/AST/OperationKinds.h"
#include "../../../../../clang/include/clang/AST/PrettyPrinter.h"
#include "../../../../../clang/include/clang/AST/RawCommentList.h"
#include "../../../../../clang/include/clang/AST/SYCLKernelInfo.h"
#include "../../../../../clang/include/clang/AST/Stmt.h"
#include "../../../../../clang/include/clang/AST/TypeBase.h"
#include "../../../../../clang/include/clang/AST/TypeLoc.h"
#include "../../../../../clang/include/clang/AST/TypeOrdering.h"
#include "../../../../../clang/include/clang/Basic/AttrKinds.h"
#include "../../../../../clang/include/clang/Basic/AttributeCommonInfo.h"
#include "../../../../../clang/include/clang/Basic/BuiltinTraits.h"
#include "../../../../../clang/include/clang/Basic/Builtins.h"
#include "../../../../../clang/include/clang/Basic/CFProtectionOptions.h"
#include "../../../../../clang/include/clang/Basic/CodeGenOptions.h"
#include "../../../../../clang/include/clang/Basic/CustomizableOptional.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticCategories.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticIDs.h"
#include "../../../../../clang/include/clang/Basic/DiagnosticOptions.h"
#include "../../../../../clang/include/clang/Basic/DirectoryEntry.h"
#include "../../../../../clang/include/clang/Basic/FileEntry.h"
#include "../../../../../clang/include/clang/Basic/FileManager.h"
#include "../../../../../clang/include/clang/Basic/FileSystemOptions.h"
#include "../../../../../clang/include/clang/Basic/HeaderInclude.h"
#include "../../../../../clang/include/clang/Basic/LangOptions.h"
#include "../../../../../clang/include/clang/Basic/LangStandard.h"
#include "../../../../../clang/include/clang/Basic/MakeSupport.h"
#include "../../../../../clang/include/clang/Basic/OpenCLOptions.h"
#include "../../../../../clang/include/clang/Basic/OpenMPKinds.h"
#include "../../../../../clang/include/clang/Basic/OperatorKinds.h"
#include "../../../../../clang/include/clang/Basic/OptionalUnsigned.h"
#include "../../../../../clang/include/clang/Basic/PragmaKinds.h"
#include "../../../../../clang/include/clang/Basic/Sanitizers.h"
#include "../../../../../clang/include/clang/Basic/SourceManager.h"
#include "../../../../../clang/include/clang/Basic/TargetCXXABI.h"
#include "../../../../../clang/include/clang/Basic/TargetInfo.h"
#include "../../../../../clang/include/clang/Basic/TokenKinds.h"
#include "../../../../../clang/include/clang/CodeGen/ModuleLinker.h"
#include "../../../../../clang/include/clang/Frontend/CommandLineSourceLoc.h"
#include "../../../../../clang/include/clang/Frontend/CompilerInvocation.h"
#include "../../../../../clang/include/clang/Frontend/FrontendAction.h"
#include "../../../../../clang/include/clang/Frontend/FrontendOptions.h"
#include "../../../../../clang/include/clang/Frontend/MigratorOptions.h"
#include "../../../../../clang/include/clang/Frontend/PreprocessorOutputOptions.h"
#include "../../../../../clang/include/clang/Frontend/Utils.h"
#include "../../../../../clang/include/clang/Lex/DependencyDirectivesScanner.h"
#include "../../../../../clang/include/clang/Lex/DirectoryLookup.h"
#include "../../../../../clang/include/clang/Lex/ExternalPreprocessorSource.h"
#include "../../../../../clang/include/clang/Lex/HeaderMap.h"
#include "../../../../../clang/include/clang/Lex/HeaderMapTypes.h"
#include "../../../../../clang/include/clang/Lex/HeaderSearch.h"
#include "../../../../../clang/include/clang/Lex/HeaderSearchOptions.h"
#include "../../../../../clang/include/clang/Lex/MacroBase.h"
#include "../../../../../clang/include/clang/Lex/ModuleLoader.h"
#include "../../../../../clang/include/clang/Lex/ModuleMap.h"
#include "../../../../../clang/include/clang/Lex/ModuleMapFile.h"
#include "../../../../../clang/include/clang/Sema/CodeCompleteOptions.h"
#include "../../../../../clang/include/clang/Serialization/ModuleFileExtension.h"
#include "../../../../../clang/include/clang/Serialization/PCHContainerOperations.h"
#include "../../../../../llvm/include/llvm/ADT/APFixedPoint.h"
#include "../../../../../llvm/include/llvm/ADT/PagedVector.h"
#include "../../../../../llvm/include/llvm/Frontend/HLSL/HLSLRootSignature.h"
#include "../../../../../llvm/include/llvm/Option/OptSpecifier.h"
#include "../../../../../llvm/include/llvm/Support/BuryPointer.h"
#include "../../../../../llvm/include/llvm/Support/ConvertUTF.h"
#include "../../../../../llvm/include/llvm/Support/VirtualOutputBackend.h"
#include "../../../../../llvm/include/llvm/TargetParser/AtomicScope.h"
#include "ASTStructExtractor.h"
#include "ClangExpressionParser.h"

#include "clang/AST/RecordLayout.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"

#include "Plugins/TypeSystem/Clang/TypeSystemClang.h"
#include "lldb/Core/Module.h"
#include "lldb/Expression/IRExecutionUnit.h"
#include "lldb/Interpreter/CommandReturnObject.h"
#include "lldb/Symbol/Function.h"
#include "lldb/Target/ThreadPlanCallFunction.h"
#include "lldb/Utility/State.h"

using namespace lldb_private;

char ClangFunctionCaller::ID;

// ClangFunctionCaller constructor
ClangFunctionCaller::ClangFunctionCaller(ExecutionContextScope &exe_scope,
                                         const CompilerType &return_type,
                                         const Address &functionAddress,
                                         const ValueList &arg_value_list,
                                         const char *name)
    : FunctionCaller(exe_scope, return_type, functionAddress, arg_value_list,
                     name),
      m_type_system_helper(*this) {
  m_jit_process_wp = lldb::ProcessWP(exe_scope.CalculateProcess());
  // Can't make a ClangFunctionCaller without a process.
  assert(m_jit_process_wp.lock());
}

// Destructor
ClangFunctionCaller::~ClangFunctionCaller() = default;

unsigned

ClangFunctionCaller::CompileFunction(lldb::ThreadSP thread_to_use_sp,
                                     DiagnosticManager &diagnostic_manager) {
  if (m_compiled)
    return 0;

  // Compilation might call code, make sure to keep on the thread the caller
  // indicated.
  ThreadList::ExpressionExecutionThreadPusher execution_thread_pusher(
      thread_to_use_sp);

  // FIXME: How does clang tell us there's no return value?  We need to handle
  // that case.
  unsigned num_errors = 0;

  std::string return_type_str(
      m_function_return_type.GetTypeName().AsCString(""));

  // Cons up the function we're going to wrap our call in, then compile it...
  // We declare the function "extern "C"" because the compiler might be in C++
  // mode which would mangle the name and then we couldn't find it again...
  m_wrapper_function_text.clear();
  m_wrapper_function_text.append("extern \"C\" void ");
  m_wrapper_function_text.append(m_wrapper_function_name);
  m_wrapper_function_text.append(" (void *input)\n{\n    struct ");
  m_wrapper_function_text.append(m_wrapper_struct_name);
  m_wrapper_function_text.append(" \n  {\n");
  m_wrapper_function_text.append("    ");
  m_wrapper_function_text.append(return_type_str);
  m_wrapper_function_text.append(" (*fn_ptr) (");

  // Get the number of arguments.  If we have a function type and it is
  // prototyped, trust that, otherwise use the values we were given.

  // FIXME: This will need to be extended to handle Variadic functions.  We'll
  // need
  // to pull the defined arguments out of the function, then add the types from
  // the arguments list for the variable arguments.

  uint32_t num_args = UINT32_MAX;
  bool trust_function = false;
  // GetArgumentCount returns -1 for an unprototyped function.
  CompilerType function_clang_type;
  if (m_function_ptr) {
    function_clang_type = m_function_ptr->GetCompilerType();
    if (function_clang_type) {
      int num_func_args = function_clang_type.GetFunctionArgumentCount();
      if (num_func_args >= 0) {
        trust_function = true;
        num_args = num_func_args;
      }
    }
  }

  if (num_args == UINT32_MAX)
    num_args = m_arg_values.GetSize();

  std::string args_buffer; // This one stores the definition of all the args in
                           // "struct caller".
  std::string args_list_buffer; // This one stores the argument list called from
                                // the structure.
  for (size_t i = 0; i < num_args; i++) {
    std::string type_name;

    if (trust_function) {
      type_name = function_clang_type.GetFunctionArgumentTypeAtIndex(i)
                      .GetTypeName()
                      .AsCString("");
    } else {
      CompilerType clang_qual_type =
          m_arg_values.GetValueAtIndex(i)->GetCompilerType();
      if (clang_qual_type) {
        type_name = clang_qual_type.GetTypeName().AsCString("");
      } else {
        diagnostic_manager.Printf(
            lldb::eSeverityError,
            "Could not determine type of input value %" PRIu64 ".",
            (uint64_t)i);
        return 1;
      }
    }

    m_wrapper_function_text.append(type_name);
    if (i < num_args - 1)
      m_wrapper_function_text.append(", ");

    char arg_buf[32];
    args_buffer.append("    ");
    args_buffer.append(type_name);
    snprintf(arg_buf, 31, "arg_%" PRIu64, (uint64_t)i);
    args_buffer.push_back(' ');
    args_buffer.append(arg_buf);
    args_buffer.append(";\n");

    args_list_buffer.append("__lldb_fn_data->");
    args_list_buffer.append(arg_buf);
    if (i < num_args - 1)
      args_list_buffer.append(", ");
  }
  m_wrapper_function_text.append(
      ");\n"); // Close off the function calling prototype.

  m_wrapper_function_text.append(args_buffer);

  m_wrapper_function_text.append("    ");
  m_wrapper_function_text.append(return_type_str);
  m_wrapper_function_text.append(" return_value;");
  m_wrapper_function_text.append("\n  };\n  struct ");
  m_wrapper_function_text.append(m_wrapper_struct_name);
  m_wrapper_function_text.append("* __lldb_fn_data = (struct ");
  m_wrapper_function_text.append(m_wrapper_struct_name);
  m_wrapper_function_text.append(" *) input;\n");

  m_wrapper_function_text.append(
      "  __lldb_fn_data->return_value = __lldb_fn_data->fn_ptr (");
  m_wrapper_function_text.append(args_list_buffer);
  m_wrapper_function_text.append(");\n}\n");

  Log *log = GetLog(LLDBLog::Expressions);
  LLDB_LOGF(log, "Expression: \n\n%s\n\n", m_wrapper_function_text.c_str());

  // Okay, now compile this expression

  lldb::ProcessSP jit_process_sp(m_jit_process_wp.lock());
  if (jit_process_sp) {
    // We will be passing in unauthenticated function addresses to the
    // FunctionCaller code, so we need to force disable pointer auth
    // codegen for this one code snippet.
    const bool force_disable_ptrauth_codegen = true;
    const bool generate_debug_info = true;
    auto *clang_parser = new ClangExpressionParser(
        jit_process_sp.get(), *this, generate_debug_info, diagnostic_manager,
        std::vector<std::string>(), "<clang expression>",
        force_disable_ptrauth_codegen);
    num_errors = clang_parser->Parse(diagnostic_manager);
    m_parser.reset(clang_parser);
  } else {
    diagnostic_manager.PutString(lldb::eSeverityError,
                                 "no process - unable to inject function");
    num_errors = 1;
  }

  m_compiled = (num_errors == 0);

  if (!m_compiled)
    return num_errors;

  return num_errors;
}

char ClangFunctionCaller::ClangFunctionCallerHelper::ID;

clang::ASTConsumer *
ClangFunctionCaller::ClangFunctionCallerHelper::ASTTransformer(
    clang::ASTConsumer *passthrough) {
  m_struct_extractor = std::make_unique<ASTStructExtractor>(
      passthrough, m_owner.GetWrapperStructName(), m_owner);

  return m_struct_extractor.get();
}
