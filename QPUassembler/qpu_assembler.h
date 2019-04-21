#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void disassemble_qpu_asm(uint64_t instruction);
void assemble_qpu_asm(char* str, uint64_t* instructions);

#ifdef __cplusplus
}
#endif
