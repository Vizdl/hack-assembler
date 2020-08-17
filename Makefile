include Makefile-extras

# must specify make target
all: test

notest: parser translator disassembler

.PHONY: parser translator disassembler

# C++ 11
CXXFLAGS=--std=c++11 -I. -Iincludes

clean:
	rm -rf lib/*/parser lib/*/translator lib/*/disassembler

parser: lib/$(CS_ARCH)/parser

lib/$(CS_ARCH)/parser: parser.cpp lib/$(CS_ARCH)/lib.a
	${CXX} ${CXXFLAGS} -o $@ $^

translator: lib/$(CS_ARCH)/translator

lib/$(CS_ARCH)/translator: translator.cpp lib/$(CS_ARCH)/lib.a
	${CXX} ${CXXFLAGS} -o $@ $^

disassembler: lib/$(CS_ARCH)/disassembler

lib/$(CS_ARCH)/disassembler: disassembler.cpp lib/$(CS_ARCH)/lib.a
	${CXX} ${CXXFLAGS} -o $@ $^

test: test-parser test-translator test-parser-translator test-disassembler

test-parser: parser
	-@bash bin/run-tests parser loud

test-translator: translator
	-@bash bin/run-tests translator loud

test-parser-translator: parser translator
	-@bash bin/run-tests parser-translator loud

test-disassembler: disassembler
	-@bash bin/run-tests disassembler-N loud
	-@bash bin/run-tests disassembler loud

test-working: test-working-parser test-working-translator test-working-disassembler

test-working-parser:
	-@bash bin/run-tests working-parser loud

test-working-translator: 
	-@bash bin/run-tests working-translator loud

test-working-disassembler: 
	-@bash bin/run-tests working-disassembler-N loud
	-@bash bin/run-tests working-disassembler loud

test-new: test-new-Pxml test-new-Pxml-hack test-new-asm

test-new-Pxml:
	-@bash bin/run-tests new-Pxml quiet

test-new-Pxml-hack:
	-@bash bin/run-tests new-Pxml-hack quiet

test-new-asm:
	-@bash bin/run-tests new-asm quiet

test-add: test-add-Pxml test-add-Pxml-hack test-add-asm

test-add-Pxml:
	-@bash bin/run-tests add-Pxml quiet

test-add-Pxml-hack:
	-@bash bin/run-tests add-Pxml-hack quiet

test-add-asm:
	-@bash bin/run-tests add-asm quiet

