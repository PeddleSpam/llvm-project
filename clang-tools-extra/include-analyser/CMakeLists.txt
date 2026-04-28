set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  Support
  )

add_clang_executable(include-analyser
  IAConsumer.cpp
  IAFrontendAction.cpp
  IAVisitor.cpp
  IncludeAnalyser.cpp
  InclusionCollector.cpp
  )

clang_target_link_libraries(include-analyser
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangTooling
  clangToolingRefactoring
  )