set(LLVM_NO_RTTI 1)

add_clang_library(clangFrontend
  ASTConsumers.cpp
  ASTMerge.cpp
  ASTUnit.cpp
  BoostConAction.cpp
  CacheTokens.cpp
  CompilerInstance.cpp
  CompilerInvocation.cpp
  DeclXML.cpp
  DependencyFile.cpp
  DiagChecker.cpp
  DocumentXML.cpp
  ExecuteCompilerInvocation.cpp
  FrontendAction.cpp
  FrontendActions.cpp
  FrontendOptions.cpp
  GeneratePCH.cpp
  InitHeaderSearch.cpp
  InitPreprocessor.cpp
  LangStandards.cpp
  PCHReader.cpp
  PCHReaderDecl.cpp
  PCHReaderStmt.cpp
  PCHWriter.cpp
  PCHWriterDecl.cpp
  PCHWriterStmt.cpp
  PrintPreprocessedOutput.cpp
  StmtXML.cpp
  TextDiagnosticBuffer.cpp
  TextDiagnosticPrinter.cpp
  TypeXML.cpp
  VerifyDiagnosticsClient.cpp
  Warnings.cpp
  )

IF(MSVC)
  get_target_property(NON_ANSI_COMPILE_FLAGS clangFrontend COMPILE_FLAGS)
  string(REPLACE /Za
    "" NON_ANSI_COMPILE_FLAGS
    ${NON_ANSI_COMPILE_FLAGS})
  set_target_properties(clangFrontend PROPERTIES COMPILE_FLAGS ${NON_ANSI_COMPILE_FLAGS})
ENDIF(MSVC)

add_dependencies(clangFrontend 
  ClangAttrClasses
  ClangAttrList
  ClangDiagnosticFrontend 
  ClangDiagnosticLex
  ClangDiagnosticSema
  ClangDeclNodes
  ClangStmtNodes)
