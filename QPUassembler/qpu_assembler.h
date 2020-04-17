#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "vc4_qpu_enums.h"

uint64_t encode_alu(qpu_sig_bits sig_bits,
					qpu_unpack unpack_mode,
					//If the pm bit is set, the unpack field programs the r4 unpack unit,
					//and the pack field is used to program the color
					//conversion on the output of the mul unit
					uint8_t pack_unpack_select,
					uint8_t pack_mode,
					qpu_cond add_cond,
					qpu_cond mul_cond,
					uint8_t set_flags, //Flags are updated from the add ALU unless the add ALU performed a NOP (or its condition code was NEVER) in which case flags are updated from the mul ALU
					uint8_t write_swap_flag, //0: add writes to A, mul to B, 1: add writes to B, mul to A
					qpu_waddr waddr_add,
					qpu_waddr waddr_mul,
					qpu_op_add op_add,
					qpu_op_mul op_mul,
					qpu_raddr raddr_a,
					qpu_raddr raddr_b,
					qpu_mux add_a,
					qpu_mux add_b,
					qpu_mux mul_a,
					qpu_mux mul_b
					);
uint64_t encode_alu_small_imm(qpu_unpack unpack_mode,
							  uint8_t pack_unpack_select,
							  uint8_t pack_mode,
							  qpu_cond add_cond,
							  qpu_cond mul_cond,
							  uint8_t set_flags, //Flags are updated from the add ALU unless the add ALU performed a NOP (or its condition code was NEVER) in which case flags are updated from the mul ALU
							  uint8_t write_swap_flag, //0: add writes to A, mul to B, 1: add writes to B, mul to A
							  qpu_waddr waddr_add,
							  qpu_waddr waddr_mul,
							  qpu_op_add op_add,
							  qpu_op_mul op_mul,
							  qpu_raddr raddr_a,
							  uint8_t small_imm,
							  qpu_mux add_a,
							  qpu_mux add_b,
							  qpu_mux mul_a,
							  qpu_mux mul_b
		);
uint64_t encode_branch(qpu_branch_cond branch_cond,
					   uint8_t is_relative, //if set branch target is relative to PC+4
					   uint8_t use_raddr_a, //if set add value of raddr_a (from simd elem 0) to branch target
					   qpu_raddr raddr_a,
					   uint8_t write_swap_bit,
					   qpu_waddr waddr_add,
					   qpu_waddr waddr_mul,
					   uint32_t imm //always added to branch target, set to 0 if unused
					   );
uint64_t encode_semaphore(uint8_t pack_unpack_select,
						  uint8_t pack_mode,
						  qpu_cond cond_add,
						  qpu_cond cond_mul,
						  uint8_t set_flags,
						  uint8_t write_swap,
						  qpu_waddr waddr_add,
						  qpu_waddr waddr_mul,
						  uint8_t incr_sem, //if 1 increment semaphore
						  uint8_t sem, //4 bit semaphore selector
						  uint32_t imm_val //27bit immediate value loaded into all 16 simd elements
						  );
uint64_t encode_load_imm(uint8_t pack_unpack_select,
						 uint8_t pack_mode,
						 qpu_cond cond_add,
						 qpu_cond cond_mul,
						 uint8_t set_flags,
						 uint8_t write_swap,
						 qpu_waddr waddr_add,
						 qpu_waddr waddr_mul,
						 uint32_t imm //2x16bit or 1x32bit uint
		);
uint64_t encode_load_imm_per_elem(
						 uint8_t signed_or_unsigned, //1 for signed, 0 for unsigned
						 uint8_t pack_unpack_select,
						 uint8_t pack_mode,
						 qpu_cond cond_add,
						 qpu_cond cond_mul,
						 uint8_t set_flags,
						 uint8_t write_swap,
						 qpu_waddr waddr_add,
						 qpu_waddr waddr_mul,
						 uint16_t ms_bit, //per element MS (sign) bit
						 uint16_t ls_bit //per element LS bit
		);
void disassemble_qpu_asm(uint64_t instruction);
void assemble_qpu_asm(char* str, uint64_t* instructions);
unsigned get_num_instructions(char* ptr);

#ifdef __cplusplus
}
#endif
