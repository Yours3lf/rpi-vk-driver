cmake_minimum_required(VERSION 2.8)

cmake_policy(SET CMP0048 NEW)

project(QPUassembler)

file(GLOB asmSrc
	"qpu_assembler.h"
	"qpu_assembler.c"
)

add_library(QPUassembler OBJECT ${asmSrc})
target_compile_options(QPUassembler PRIVATE -Wall -Werror=implicit-function-declaration)

add_executable(QPUassemblerExe ${asmSrc} main.c shaders.h)
target_compile_options(QPUassemblerExe PRIVATE -Wall -Werror=implicit-function-declaration)

install(TARGETS QPUassemblerExe RUNTIME DESTINATION bin)
