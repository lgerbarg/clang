##===- tools/index-test/Makefile ---------------------------*- Makefile -*-===##
# 
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
# 
##===----------------------------------------------------------------------===##
CLANG_LEVEL := ../..

TOOLNAME = c-index-test

# No plugins, optimize startup time.
TOOL_NO_EXPORTS = 1

LINK_COMPONENTS := bitreader mc core
USEDLIBS = clang.a clangIndex.a clangFrontend.a clangDriver.a clangSema.a \
	   clangAnalysis.a clangAST.a clangParse.a clangLex.a clangBasic.a

include $(CLANG_LEVEL)/Makefile
