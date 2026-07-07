set(LLVM_LINK_COMPONENTS Support)

add_clang_executable(include-analyser
  IAActionFactory.cpp
  IAConsumer.cpp
  IAFrontendAction.cpp
  IAVisitor.cpp
  IncludeAnalyser.cpp
  InclusionCollector.cpp
  IAOptions.cpp
  )

clang_target_link_libraries(include-analyser
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFormat
  clangFrontend
  clangLex
  clangSerialization
  clangTooling
  clangToolingRefactoring
  )
