#pragma once

#include <stdint.h>

void disassemble_qpu_asm(uint64_t instruction);
void assemble_qpu_asm(char* str, uint64_t* instructions);
