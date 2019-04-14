cmake_minimum_required(VERSION 2.8)

project(QPUassembler)

file(GLOB asmSrc
	"*.h"
	"*.c"
)

add_executable(${PROJECT_NAME} ${asmSrc})
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror=implicit-function-declaration -std=c11)
