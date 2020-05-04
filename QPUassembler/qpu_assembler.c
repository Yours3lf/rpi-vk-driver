#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qpu_assembler.h"
#include "vc4_qpu_defines.h"

/*********************************************************************************************************************
	Instruction restrictions

	* The last three instructions of any program (Thread End plus the following two delay-slot instructions) must
		not do varyings read, uniforms read or any kind of VPM, VDR, or VDW read or write.
	* The Program End instruction must not write to either physical regfile A or B.
	* The Program End instruction and the following two delay slot instructions must not write or read address 14
		in either regfile A or B.
	* The final program instruction (the second delay slot instruction) must not do a TLB Z write.
	* A scoreboard wait must not occur in the first two instructions of a fragment shader. This is either the
		explicit Wait for Scoreboard signal or an implicit wait with the first tile-buffer read or write instruction.
	* If TMU_NOSWAP is written, the write must be three instructions before the first TMU write instruction.
		For example, if TMU_NOSWAP is written in the first shader instruction, the first TMU write cannot occur
		before the 4th shader instruction.
	* An instruction must not read from a location in physical regfile A or B that was written to by the previous
		instruction.
	* After an SFU lookup instruction, accumulator r4 must not be read in the following two instructions. Any
		other instruction that results in r4 being written (that is, TMU read, TLB read, SFU lookup) cannot occur in
		the two instructions following an SFU lookup.
	* An instruction that does a vector rotate by r5 must not immediately follow an instruction that writes to r5.
	* An instruction that does a vector rotate must not immediately follow an instruction that writes to the
		accumulator that is being rotated.
	* After an instruction that does a TLB Z write, the multisample mask must not be read as an instruction
		input argument in the following two instruction. The TLB Z write instruction can, however, be followed
		immediately by a TLB color write.
	* A single instruction can only perform a maximum of one of the following closely coupled peripheral
		accesses in a single instruction: TMU write, TMU read, TLB write, TLB read, TLB combined color read and
		write, SFU write, Mutex read or Semaphore access.
 *********************************************************************************************************************/

/*
Format:
#comment
sig_bit_opt		; dstAdd.pack_opt	= add_op.pm_opt.sf_opt.cond.unpack_opt.ws_opt(srcA, srcB, raddr_a_opt, raddr_b_opt)	; dstMul.pack_opt	= mul_op.cond(srcA, srcB)	;
sig_small_imm	; dstAdd.pack_opt	= add_op.pm_opt.sf_opt.cond.unpack_opt.ws_opt(srcA, srcB, raddr_a_opt, small_imm)	; dstMul.pack_opt	= mul_op.cond(srcA, srcB)	;
sig_branch		; dstAdd			= branch.rel_opt.reg_opt.ws_opt(address, condition, raddr_a_opt)					; dstMul			= branch()					;
sig_load_imm	; dstAdd.pack_opt	= sem_inc.pm_opt.sf_opt.cond.ws_opt(sem_number, 27bit_imm_opt)						; dstMul.pack_opt	= sem_inc.cond()			;
sig_load_imm	; dstAdd.pack_opt	= load32.pm_opt.sf_opt.cond.ws_opt(immediate32bit_value)							; dstMul.pack_opt	= load32.cond()				;
sig_load_imm	; dstAdd.pack_opt	= load16.pm_opt.signed_opt.sf_opt.cond.ws_opt(int16_imm, int16_imm)					; dstMul.pack_opt	= load16.cond()				;

==================================================================
================How to formulate instructions:====================
==================================================================
1)
You must specify the signal bits at the beginning of each instruction:
sig_brk, sig_none, sig_switch, sig_end, sig_wait_score, sig_unlock_score, sig_thread_switch, sig_coverage_load,
sig_color_load, sig_color_load_end, sig_load_tmu0, sig_load_tmu1, sig_alpha_mask_load, sig_small_imm, sig_load_imm, sig_branch

2)
Then you must specify the output register for the ADD pipeline.
rx0-31, r0-3, r5, tmu_noswap, host_int, nop, uniforms_addr, quad_x, quad_y, ms_flags, rev_flags, tlb_stencil_setup
tlb_z, tlb_color_ms, tlb_color_all, vpm, vr_setup, vr_addr, mutex_release, sfu_recip, sfu_recipsqrt, sfu_exp,
sfu_log, tmu0_s, tmu0_t, tmu0_r, tmu0_b, tmu1_s, tmu1_t, tmu1_r, tmu1_b

3)
If the ADD instruction writes to regfile A (ie. you don't specify the WS flag later) and PM flag won't be specified,
then you can specify the pack mode for regfile A here (omitting means nop)
nop, 16a, 16b, 8888, 8a, 8b, 8c, 8d, sta, 16a.sat, 16b.sat, 8888.sat, 8a.sat, 8b.sat, 8c.sat, 8d.sat

4)
Then you must specify your operation for the ADD pipeline. If you are writing a non-ALU instruction, you can specify either
branch, sem_inc, sem_dec, load32 or load16 here instead.
Operations available:
nop, fadd, fsub, fmin, fmax, fminabs, fmaxabs, ftoi, itof, add, sub, shr, asr, ror, shl, min, max, and, or, xor, not, clz, v8adds, v8subs

5)
Then you can specify a range of modifiers (order is not important):
PM bit: pm
SF bit: sf
WS bit: ws
REL bit: rel
REG bit: reg
SIGNED bit: signed
Conditional execution for the ADD pipeline (default is never): never, always, zs, zc, ns, nc, cs, cc
Unpack modes (from regfile A, or if PM is set from R4): nop, 16a, 16b, 8d_rep, 8a, 8b, 8c, 8d

6)
Then you must specify the arguments for the ALU operation.
srcA, srcB can be: r0-r5 or a, b for regfiles A and B, or imm for the small immediate value.
raddr_a and raddr_b can be specified afterwards as optional extra arguments (omitting means nop).
raddr_a: ra0-31, pay_zw, uni, vary, elem, nop, x_pix, ms_flags, vpm_read, vpm_ld_busy, vpm_ld_wait, mutex_acq
raddr_b: rb0-31, pay_zw, uni, vary, elem, nop, y_pix, rev_flag, vpm_read, vpm_st_busy, vpm_st_wait, mutex_acq

For branch operation, you must specify:
the jump address as a 32bit value (can be relative if REL is set)
the branch condition: all_zs, all_zc, any_zs, any_zc, all_ns, all_nc, any_ns, any_nc, all_cs, all_cc, any_cs, any_cc, always
and an optional raddr_a (if REG flag is set), see above

For a semaphore instruction, you need to specify which semaphore (0-15) you want to modify, then an optional 27bit immediate value (ms 16bits might be usable...).

7)
Then you must specify the output register for the MUL pipeline.
See above for options.

8)
If the MUL instruction writes to regfile A (ie. you specify the WS flag) then you can set the pack operation for regfile A here:
nop, 16a, 16b, 8888, 8a, 8b, 8c, 8d, sta, 16a.sat, 16b.sat, 8888.sat, 8a.sat, 8b.sat, 8c.sat, 8d.sat
OR
You if specify the PM flag, then you can set the pack operation for the MUL output here:
nop, 8888, 8a, 8b, 8c, 8d

9)
Then you must specify your operation for the MUL pipeline. If you are writing a non-ALU instruction, you can specify either
branch, sem_inc, sem_dec, load32 or load16 here instead.
Operations available:
nop, fmul, mul24, v8muld, v8min, v8max, v8adds, v8subs

10)
Then you can specify a range of modifiers (order is not important):
Conditional execution for the MUL pipeline: never, always, zs, zc, ns, nc, cs, cc

11)
Then you must specify the arguments for the ALU operation.
srcA, srcB can be: r0-r5 or a, b for regfiles A and B, or imm for the small immediate value.

==================================================================
==================================================================

Examples:
sig_none		; rx0.nop			= add.pm.sf.always(r0, r1, 0)														; rx0.nop					= fmul.always(r2, r3)	;
sig_branch		; rx0				= branch.pm.rel.reg.always(0xdeadbeef, ra1)											; rx0						= branch()				;
sig_none		; rx0.nop			= sem_inc.pm.sf.always(1, 0x7ffffff)												; rx0.nop					= sem_inc.always()		;
sig_load_imm	; rx0.nop			= load32.pm.sf.always(0xdeadbeef)													; rx0.nop					= load32.always()		;
sig_load_imm	; rx0.nop			= load16.pm.sf.signed.always(1, 2)													; rx0.nop					= load16.always()		;
#mov
sig_none		; rx0.nop			= or(r0, r0)																		; rx0						= v8min(r1, r1)			;
#nop
sig_none		; nop				= nop(nop, nop)																		; nop						= nop(nop, nop)			;
 */

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
					)
{
	uint64_t res = 0;
	uint64_t tmp = 0;

	tmp = sig_bits & 0xf; //mask ls 4 bits
	res |= tmp << QPU_SIG_SHIFT;

	tmp = unpack_mode & 0x7; //mask ls 3 bits
	res |= tmp << QPU_UNPACK_SHIFT;

	tmp = pack_unpack_select & 1;
	res |= tmp << 56;

	tmp = pack_mode & 0xf;
	res |= tmp << QPU_PACK_SHIFT;

	tmp = add_cond & 0x7;
	res |= tmp << QPU_COND_ADD_SHIFT;

	tmp = mul_cond & 0x7;
	res |= tmp << QPU_COND_MUL_SHIFT;

	tmp = set_flags & 1;
	res |= tmp << 45;

	tmp = write_swap_flag & 1;
	res |= tmp << 44;

	tmp = waddr_add & 0x3f;
	res |= tmp << QPU_WADDR_ADD_SHIFT;

	tmp = waddr_mul & 0x3f;
	res |= tmp << QPU_WADDR_MUL_SHIFT;

	tmp = op_mul & 0x7;
	res |= tmp << QPU_OP_MUL_SHIFT;

	tmp = op_add & 0x1f;
	res |= tmp << QPU_OP_ADD_SHIFT;

	tmp = raddr_a & 0x3f;
	res |= tmp << QPU_RADDR_A_SHIFT;

	tmp = raddr_b & 0x3f;
	res |= tmp << QPU_RADDR_B_SHIFT;

	tmp = add_a & 0x7;
	res |= tmp << QPU_ADD_A_SHIFT;

	tmp = add_b & 0x7;
	res |= tmp << QPU_ADD_B_SHIFT;

	tmp = mul_a & 0x7;
	res |= tmp << QPU_MUL_A_SHIFT;

	tmp = mul_b & 0x7;
	res |= tmp << QPU_MUL_B_SHIFT;

	return res;
}

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
		)
{
	return encode_alu(0xd,
					  unpack_mode,
					  pack_unpack_select,
					  pack_mode,
					  add_cond,
					  mul_cond,
					  set_flags,
					  write_swap_flag,
					  waddr_add,
					  waddr_mul,
					  op_add,
					  op_mul,
					  raddr_a,
					  small_imm,
					  add_a,
					  add_b,
					  mul_a,
					  mul_b);
}

uint64_t encode_branch(qpu_branch_cond branch_cond,
					   uint8_t is_relative, //if set branch target is relative to PC+4
					   uint8_t use_raddr_a, //if set add value of raddr_a (from simd elem 0) to branch target
					   qpu_raddr raddr_a,
					   uint8_t write_swap_bit,
					   qpu_waddr waddr_add,
					   qpu_waddr waddr_mul,
					   uint32_t imm //always added to branch target, set to 0 if unused
					   )
{
	uint64_t res = 0;
	uint64_t tmp = 0;

	tmp = 0xf;
	res |= tmp << 60;

	tmp = branch_cond & 0xf;
	res |= tmp << QPU_BRANCH_COND_SHIFT;

	tmp = is_relative & 1;
	res |= tmp << 51;

	tmp = use_raddr_a & 1;
	res |= tmp << 50;

	tmp = raddr_a & 0x1f;
	res |= tmp << QPU_BRANCH_RADDR_A_SHIFT;

	tmp = write_swap_bit & 1;
	res |= tmp << 44;

	tmp = waddr_add & 0x3f;
	res |= tmp << QPU_WADDR_ADD_SHIFT;

	tmp = waddr_mul & 0x3f;
	res |= tmp << QPU_WADDR_MUL_SHIFT;

	res |= imm;

	return res;
}

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
						  )
{
	uint64_t res = 0;
	uint64_t tmp = 0;

	tmp = 0x74;
	res |= tmp << 57;

	tmp = pack_unpack_select & 1;
	res |= tmp << 56;

	tmp = pack_mode & 0xf;
	res |= tmp << QPU_PACK_SHIFT;

	tmp = cond_add & 0x7;
	res |= tmp << QPU_COND_ADD_SHIFT;

	tmp = cond_mul & 0x7;
	res |= tmp << QPU_COND_MUL_SHIFT;

	tmp = set_flags & 1;
	res |= tmp << 45;

	tmp = write_swap & 1;
	res |= tmp << 44;

	tmp = waddr_add & 0x3f;
	res |= tmp << QPU_WADDR_ADD_SHIFT;

	tmp = waddr_mul & 0x3f;
	res |= tmp << QPU_WADDR_MUL_SHIFT;

	tmp = imm_val & 0x7ffffff;
	res |= tmp << 5;

	tmp = incr_sem & 1;
	res |= tmp << 4;

	res |= sem & 0xf;

	return res;
}

//write immediate value across simd array
uint64_t encode_load_imm(uint8_t pack_unpack_select,
						 uint8_t pack_mode,
						 qpu_cond cond_add,
						 qpu_cond cond_mul,
						 uint8_t set_flags,
						 uint8_t write_swap,
						 qpu_waddr waddr_add,
						 qpu_waddr waddr_mul,
						 uint32_t imm //2x16bit or 1x32bit uint
		)
{
	uint64_t res = 0;
	uint64_t tmp = 0;

	tmp = 0x70;
	res |= tmp << 57;

	tmp = pack_unpack_select & 1;
	res |= tmp << 56;

	tmp = pack_mode & 0xf;
	res |= tmp << QPU_PACK_SHIFT;

	tmp = cond_add & 0x7;
	res |= tmp << QPU_COND_ADD_SHIFT;

	tmp = cond_mul & 0x7;
	res |= tmp << QPU_COND_MUL_SHIFT;

	tmp = set_flags & 1;
	res |= tmp << 45;

	tmp = write_swap & 1;
	res |= tmp << 44;

	tmp = waddr_add & 0x3f;
	res |= tmp << QPU_WADDR_ADD_SHIFT;

	tmp = waddr_mul & 0x3f;
	res |= tmp << QPU_WADDR_MUL_SHIFT;

	res |= imm;

	return res;
}

//write per element MS bit and LS bit across simd array
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
		)
{
	uint64_t res = 0;
	uint64_t tmp = 0;

	tmp = 0x71;
	tmp |= !signed_or_unsigned << 1;
	res |= tmp << 57;

	tmp = pack_unpack_select & 1;
	res |= tmp << 56;

	tmp = pack_mode & 0xf;
	res |= tmp << QPU_PACK_SHIFT;

	tmp = cond_add & 0x7;
	res |= tmp << QPU_COND_ADD_SHIFT;

	tmp = cond_mul & 0x7;
	res |= tmp << QPU_COND_MUL_SHIFT;

	tmp = set_flags & 1;
	res |= tmp << 45;

	tmp = write_swap & 1;
	res |= tmp << 44;

	tmp = waddr_add & 0x3f;
	res |= tmp << QPU_WADDR_ADD_SHIFT;

	tmp = waddr_mul & 0x3f;
	res |= tmp << QPU_WADDR_MUL_SHIFT;

	tmp = ms_bit;
	res |= tmp << 16;

	res |= ls_bit;

	return res;
}

qpu_sig_bits parse_sig_bit(char* str)
{
	unsigned num_sig_bits = sizeof(qpu_sig_bits_str) / sizeof(const char *);

	for(unsigned c = 0; c < num_sig_bits && str; ++c)
	{
		if(qpu_sig_bits_str[c] && strcmp(str, qpu_sig_bits_str[c]) == 0)
		{
			return c;
		}
	}

	return -1;
}

void parse_dst(char** str, qpu_waddr* waddr, uint8_t* pack_mode, unsigned is_add, unsigned pm_set)
{
	char* dst = strtok(*str, ".");
	char* pack = strtok(0, ".");

	//advance token past dst strings so we can tokenize further
	if(dst)
	{
		if(pack)
		{
			*str = pack;
		}
		else
		{
			*str = dst;
		}

		while(**str)
		{
			(*str)++;
		}

		*str += 1;
	}

	uint8_t waddr_res = 0;
	uint8_t pack_mode_res = 0;

	for(unsigned c = 0; c < 2 && dst && !waddr_res; ++c)
	{
		for(unsigned d = 0; d < 64; ++d)
		{
			if(qpu_waddr_str[c][d] && strcmp(dst, qpu_waddr_str[c][d]) == 0)
			{
				waddr_res = d;
				break;
			}
		}
	}

	if(dst && dst[0] == 'r' && dst[1] == 'x')
	{
		waddr_res = strtoul(dst+2, 0, 0);
	}

	unsigned num_pack_a_str = sizeof(qpu_pack_a_str) / sizeof(const char *);
	for(unsigned c = 0; c < num_pack_a_str && pack && !pack_mode_res; ++c)
	{
		if(qpu_pack_a_str[c] && strcmp(pack, qpu_pack_a_str[c]) == 0)
		{
			pack_mode_res = c;
			break;
		}
	}

	unsigned num_pack_mul_str = sizeof(qpu_pack_mul_str) / sizeof(const char *);
	for(unsigned c = 0; c < num_pack_mul_str && pack && !pack_mode_res; ++c)
	{
		if(qpu_pack_mul_str[c] && strcmp(pack, qpu_pack_mul_str[c]) == 0)
		{
			pack_mode_res = c;
			break;
		}
	}

	*waddr = waddr_res;
	if(is_add || pm_set)
	{
		*pack_mode = pack_mode_res;
	}
}

void parse_op_modifiers(char** str, uint8_t* signed_or_unsigned, uint8_t* ws, uint8_t* pm, uint8_t* sf, qpu_cond* condition, qpu_unpack* unpack_mode, uint8_t* rel, uint8_t* reg, unsigned is_add)
{
	char* modifier = strtok(*str, ".");

	//at most 5 modifiers supported
	for(int c = 0; c < 5; ++c)
	{
		if(modifier)
		{
			*str = modifier;

			if(strcmp(modifier, "pm") == 0 && is_add)
			{
				*pm = 1;
				modifier = strtok(0, ".");
				continue;
			}

			if(strcmp(modifier, "ws") == 0 && is_add)
			{
				*ws = 1;
				modifier = strtok(0, ".");
				continue;
			}

			if(strcmp(modifier, "rel") == 0 && is_add)
			{
				*rel = 1;
				modifier = strtok(0, ".");
				continue;
			}

			if(strcmp(modifier, "reg") == 0 && is_add)
			{
				*reg = 1;
				modifier = strtok(0, ".");
				continue;
			}

			if(strcmp(modifier, "sf") == 0 && is_add)
			{
				*sf = 1;
				modifier = strtok(0, ".");
				continue;
			}

			if(strcmp(modifier, "signed") == 0 && is_add)
			{
				*signed_or_unsigned = 1;
				modifier = strtok(0, ".");
				continue;
			}

			unsigned found = 0;
			unsigned num_conds = sizeof(qpu_cond_str) / sizeof(const char *);

			for(unsigned d = 0; d < num_conds; ++d)
			{
				if(qpu_cond_str[d] && strcmp(modifier, qpu_cond_str[d]) == 0)
				{
					*condition = d;
					found = 1;
					break;
				}
			}

			if(found)
			{
				modifier = strtok(0, ".");
				continue;
			}

			if(is_add)
			{
				unsigned num_unpack_modes = sizeof(qpu_unpack_str) / sizeof(const char *);

				for(unsigned d = 0; d < num_unpack_modes; ++d)
				{
					if(qpu_unpack_str[d] && strcmp(modifier, qpu_unpack_str[d]) == 0)
					{
						*unpack_mode = d;
						break;
					}
				}
			}

			modifier = strtok(0, ".");
		}
	}

	//advance token past op strings so we can tokenize further
	while(**str)
	{
		(*str)++;
	}

	*str += 1;
}

void parse_op(char** str, qpu_alu_type* type, qpu_op_add* op_add, qpu_op_mul* op_mul, uint8_t* is_sem_inc, qpu_load_type* load_type, unsigned is_add)
{
	char* op = strtok(*str, ".");

	if(op && strcmp(op, "sem_inc") == 0)
	{
		*type = QPU_SEM;
		*is_sem_inc = 1;
	}
	else if(op && strcmp(op, "sem_dec") == 0)
	{
		*type = QPU_SEM;
		*is_sem_inc = 0;
	}
	else if(op && strcmp(op, "branch") == 0)
	{
		*type = QPU_BRANCH;
	}
	else if(op && strcmp(op, "load32") == 0)
	{
		*type = QPU_LOAD_IMM;
		*load_type = QPU_LOAD32;
	}
	else if(op && strcmp(op, "load16") == 0)
	{
		*type =	QPU_LOAD_IMM;
		*load_type = QPU_LOAD16;
	}
	else
	{
		*type = QPU_ALU;

		unsigned num_add_ops = sizeof(qpu_op_add_str) / sizeof(const char *);
		unsigned num_mul_ops = sizeof(qpu_op_mul_str) / sizeof(const char *);

		if(is_add)
		{
			for(unsigned c = 0; c < num_add_ops && op; ++c)
			{
				if(qpu_op_add_str[c] && strcmp(op, qpu_op_add_str[c]) == 0)
				{
					*op_add = c;
					break;
				}
			}
		}
		else
		{
			for(unsigned c = 0; c < num_mul_ops && op; ++c)
			{
				if(qpu_op_mul_str[c] && strcmp(op, qpu_op_mul_str[c]) == 0)
				{
					*op_mul = c;
					break;
				}
			}
		}
	}

	if(op)
	{
		*str = op;
	}

	//advance token past op strings so we can tokenize further
	while(**str)
	{
		(*str)++;
	}

	*str += 1;
}

void parse_args_alu(char** str, qpu_mux* in_a, qpu_mux* in_b, uint8_t* raddr_a, uint8_t* raddr_b, uint8_t is_si)
{
	char* arg = strtok(*str, " \n\v\f\r\t,");

	unsigned num_muxes = sizeof(qpu_mux_str) / sizeof(const char *);

	for(unsigned c = 0; c < num_muxes && arg; ++c)
	{
		if(qpu_mux_str[c] && strcmp(arg, qpu_mux_str[c]) == 0)
		{
			*str = arg;
			*in_a = c;
			break;
		}
	}

	arg = strtok(0, " \n\v\f\r\t,");

	for(unsigned c = 0; c < num_muxes && arg; ++c)
	{
		if(qpu_mux_str[c] && strcmp(arg, qpu_mux_str[c]) == 0)
		{
			*str = arg;
			*in_b = c;
			break;
		}
	}

	arg = strtok(0, " \n\v\f\r\t,");

	if(arg)
	{
		uint8_t raddr_a_res = 0;

		for(unsigned d = 0; d < 52; ++d)
		{
			if(qpu_raddr_str[0][d] && strcmp(arg, qpu_raddr_str[0][d]) == 0)
			{
				raddr_a_res = d;
				break;
			}
		}

		if(!raddr_a_res && arg && arg[0] == 'r' && arg[1] == 'a')
		{
			raddr_a_res = strtoul(arg+2, 0, 0);
		}


		*raddr_a = raddr_a_res;
		*str = arg;
	}

	arg = strtok(0, " \n\v\f\r\t,");

	if(arg)
	{
		uint8_t raddr_b_res = 0;

		for(unsigned c = 0; c < 2 && arg && !raddr_b_res; ++c)
		{
			for(unsigned d = 0; d < 52; ++d)
			{
				if(qpu_raddr_str[c][d] && strcmp(arg, qpu_raddr_str[c][d]) == 0)
				{
					raddr_b_res = d;
					break;
				}
			}
		}

		if(!raddr_b_res && arg && arg[0] == 'r' && arg[1] == 'b')
		{
			raddr_b_res = strtoul(arg+2, 0, 0);
		}

		if(is_si)
		{
			uint32_t si = strtoul(arg, 0, 0);
			raddr_b_res = qpu_encode_small_immediate(si);
		}

		*raddr_b = raddr_b_res;
		*str = arg;
	}

	//advance token past arg strings so we can tokenize further
	while(**str)
	{
		(*str)++;
	}

	*str += 1;
}

void parse_args_sem(char** str, uint8_t* sem, uint32_t* imm32)
{
	char* arg = strtok(*str, " \n\v\f\r\t,");

	if(arg)
	{
		*sem = strtoul(arg, 0, 0);
		*str = arg;
	}

	arg = strtok(0, " \n\v\f\r\t,");

	if(arg)
	{
		*imm32 = strtoul(arg, 0, 0);
		*str = arg;
	}

	//advance token past arg strings so we can tokenize further
	while(**str)
	{
		(*str)++;
	}

	*str += 1;
}

void parse_args_branch(char** str, uint32_t* imm32, qpu_branch_cond* branch_cond, uint8_t* raddr_a)
{
	char* arg = strtok(*str, " \n\v\f\r\t,");

	if(arg)
	{
		*imm32 = strtoul(arg, 0, 0);
		*str = arg;
	}

	arg = strtok(0, " \n\v\f\r\t,");

	if(arg)
	{
		unsigned num_branch_conds = sizeof(qpu_branch_cond_str) / sizeof(const char *);

		for(unsigned c = 0; c < num_branch_conds && arg; ++c)
		{
			if(qpu_branch_cond_str[c] && strcmp(arg, qpu_branch_cond_str[c]) == 0)
			{
				*branch_cond = c;
				*str = arg;
				break;
			}
		}
	}

	arg = strtok(0, " \n\v\f\r\t,");

	if(arg)
	{
		uint8_t raddr_a_res = 0;

		for(unsigned c = 0; c < 2 && arg && !raddr_a_res; ++c)
		{
			for(unsigned d = 0; d < 52; ++d)
			{
				if(qpu_raddr_str[c][d] && strcmp(arg, qpu_raddr_str[c][d]) == 0)
				{
					raddr_a_res = d;
					break;
				}
			}
		}

		if(!raddr_a_res && arg && arg[0] == 'r' && arg[1] == 'a')
		{
			raddr_a_res = strtoul(arg+2, 0, 0);
		}

		*raddr_a = raddr_a_res;
		*str = arg;
	}

	//advance token past arg strings so we can tokenize further
	while(**str)
	{
		(*str)++;
	}

	*str += 1;
}

void parse_args_load(char** str, qpu_load_type load_type, uint32_t* imm32, uint16_t* ms_imm16, uint16_t* ls_imm16)
{
	char* arg = strtok(*str, " \n\v\f\r\t,");

	if(load_type == QPU_LOAD32)
	{
		if(arg)
		{
			*imm32 = strtoul(arg, 0, 0);
			*str = arg;
		}
	}
	else
	{
		if(arg)
		{
			*ms_imm16 = strtoul(arg, 0, 0);
			*str = arg;
		}

		arg = strtok(0, " \n\v\f\r\t,");

		if(arg)
		{
			*ls_imm16 = strtoul(arg, 0, 0);
			*str = arg;
		}
	}

	//advance token past arg strings so we can tokenize further
	while(**str)
	{
		(*str)++;
	}

	*str += 1;
}


void assemble_qpu_asm(char* str, uint64_t* instructions)
{
	unsigned instruction_counter = 0;

	//delete lines that have comments in them
	char* comment_token = strstr(str, "#");

	while(comment_token)
	{
		while(*comment_token != '\n')
		{
			*comment_token = ' ';
			comment_token++;
		}
		*comment_token = ' ';
		comment_token = strstr(comment_token, "#");
	}


	//parse string token by token
	char* token = strtok(str, " \n\v\f\r\t;");

	while(token)
	{
		qpu_sig_bits sig_bit = QPU_SIG_NONE;
		qpu_alu_type type = QPU_ALU;
		qpu_op_add op_add =	QPU_A_NOP;
		qpu_op_mul op_mul =	QPU_M_NOP;
		qpu_mux mul_a = 0;
		qpu_mux mul_b = 0;
		qpu_mux add_a = 0;
		qpu_mux add_b = 0;
		qpu_cond cond_mul = QPU_COND_NEVER;
		qpu_cond cond_add = QPU_COND_NEVER;
		qpu_waddr waddr_add = QPU_W_NOP;
		qpu_waddr waddr_mul = QPU_W_NOP;
		qpu_waddr raddr_a = QPU_R_NOP;
		qpu_waddr raddr_b = QPU_R_NOP;
		uint8_t pack_unpack_select = 0;
		uint8_t pack_mode = QPU_PACK_A_NOP;
		qpu_unpack unpack_mode = QPU_UNPACK_NOP;
		uint8_t is_sem_inc = 0;
		uint8_t rel = 0;
		uint8_t reg = 0;
		uint8_t ws = 0;
		uint8_t sf = 0;
		uint32_t imm32 = 0;
		uint16_t ms_imm16 = 0;
		uint16_t ls_imm16 = 0;
		uint8_t semaphore = 0;
		qpu_load_type load_type = QPU_LOAD32;
		uint8_t is_signed = 0;
		qpu_branch_cond branch_cond = QPU_COND_BRANCH_ALWAYS;

		sig_bit = parse_sig_bit(token);
		if(sig_bit < 0)
		{
			break;
		}

		//get dst for add
		token = strtok(0, " \n\v\f\r\t=;");
		parse_dst(&token, &waddr_add, &pack_mode, 1, 0);

		//check op
		token = strtok(token, " \n\v\f\r\t=(");
		unsigned has_modifiers = strstr(token, ".") != 0;
		parse_op(&token, &type, &op_add, &op_mul, &is_sem_inc, &load_type, 1);

		//get modifiers
		if(has_modifiers)
		{
			//token = strtok(token, " \n\v\f\r\t");
			parse_op_modifiers(&token, &is_signed, &ws, &pack_unpack_select, &sf, &cond_add, &unpack_mode, &rel, &reg, 1);
		}

		if(type == QPU_ALU)
		{
			//get arguments for add
			token = strtok(token, ")");
			parse_args_alu(&token, &add_a, &add_b, &raddr_a, &raddr_b, sig_bit == QPU_SIG_SMALL_IMM);
		}
		else if(type == QPU_SEM)
		{
			//get arguments for sem
			token = strtok(token, ")");
			parse_args_sem(&token, &semaphore, &imm32);
		}
		else if(type == QPU_BRANCH)
		{
			//get arguments for branch
			token = strtok(token, ")");
			parse_args_branch(&token, &imm32, &branch_cond, &raddr_a);
		}
		else if(type == QPU_LOAD_IMM)
		{
			//get arguments for load imm
			token = strtok(token, ")");
			parse_args_load(&token, load_type, &imm32, &ms_imm16, &ls_imm16);
		}

		//get dst for mul
		token = strtok(token, " \n\v\f\r\t=;");
		parse_dst(&token, &waddr_mul, &pack_mode, 0, pack_unpack_select);

		//check op
		token = strtok(token, " \n\v\f\r\t=(");
		has_modifiers = strstr(token, ".") != 0;
		parse_op(&token, &type, &op_add, &op_mul, &is_sem_inc, &load_type, 0);

		//get modifiers
		if(has_modifiers)
		{
			//token = strtok(token, " \n\v\f\r\t(");
			parse_op_modifiers(&token, &is_signed, &ws, &pack_unpack_select, &sf, &cond_mul, &unpack_mode, &rel, &reg, 0);
		}

		token = strtok(token, ")");

		if(type == QPU_ALU)
		{
			//get arguments for mul
			parse_args_alu(&token, &mul_a, &mul_b, &raddr_a, &raddr_b, sig_bit == QPU_SIG_SMALL_IMM);
		}

		//EMIT INSTRUCTION HERE
		if(type == QPU_ALU)
		{
			if(sig_bit == QPU_SIG_SMALL_IMM)
			{
				instructions[instruction_counter] = encode_alu_small_imm(unpack_mode, pack_unpack_select, pack_mode, cond_add, cond_mul, sf, ws, waddr_add, waddr_mul, op_add, op_mul, raddr_a, raddr_b, add_a, add_b, mul_a, mul_b);
			}
			else
			{
				instructions[instruction_counter] = encode_alu(sig_bit, unpack_mode, pack_unpack_select, pack_mode, cond_add, cond_mul, sf, ws, waddr_add, waddr_mul, op_add, op_mul, raddr_a, raddr_b, add_a, add_b, mul_a, mul_b);
			}
		}
		else if(type == QPU_SEM)
		{
			instructions[instruction_counter] = encode_semaphore(pack_unpack_select, pack_mode, cond_add, cond_mul, sf, ws, waddr_add, waddr_mul, is_sem_inc, semaphore, imm32);
		}
		else if(type ==	QPU_BRANCH)
		{
			instructions[instruction_counter] = encode_branch(branch_cond, rel, reg, raddr_a, ws, waddr_add, waddr_mul, imm32);
		}
		else if(type == QPU_LOAD_IMM)
		{
			if(load_type ==	QPU_LOAD32)
			{
				instructions[instruction_counter] = encode_load_imm(pack_unpack_select, pack_mode, cond_add, cond_mul, sf, ws, waddr_add, waddr_mul, imm32);
			}
			else
			{
				instructions[instruction_counter] = encode_load_imm_per_elem(is_signed, pack_unpack_select, pack_mode, cond_add, cond_mul, sf, ws, waddr_add, waddr_mul, ms_imm16, ls_imm16);
			}
		}

		instruction_counter++;
		token = strtok(token, " \n\v\f\r\t;");
	}
}

void disassemble_qpu_asm(uint64_t instruction)
{
#define GET_BITFIELD(num_bits, place) (((instruction) & ((uint64_t)num_bits << place)) >> place)

	qpu_sig_bits sig_bits = GET_BITFIELD(0xf, 60);
	printf("\"%s ; ", qpu_sig_bits_str[sig_bits]);

	unsigned is_sem = GET_BITFIELD(0x7f, 57) == 0x74;

	qpu_waddr waddr_add = GET_BITFIELD(0x3f, QPU_WADDR_ADD_SHIFT);
	qpu_waddr waddr_mul = GET_BITFIELD(0x3f, QPU_WADDR_MUL_SHIFT);
	uint8_t ws = GET_BITFIELD(1, 44);
	uint8_t pm = GET_BITFIELD(1, 56);

	if(waddr_add <= 31)
	{
		printf("rx%d", waddr_add);
	}
	else
	{
		printf("%s", qpu_waddr_str[ws][waddr_add]);
	}

	if(is_sem)
	{
		uint8_t pack_mode = GET_BITFIELD(0xf, QPU_PACK_SHIFT);

		if(!ws && !pm)
		{
			printf(".%s", qpu_pack_a_str[pack_mode]);
		}

		uint8_t is_sem_inc = GET_BITFIELD(1, 4);

		printf(" = %s", is_sem_inc ? "sem_inc" : "sem_dec");

		if(ws)
		{
			printf(".ws");
		}

		if(pm)
		{
			printf(".pm");
		}

		qpu_cond cond_add = GET_BITFIELD(0x7, QPU_COND_ADD_SHIFT);

		printf(".%s", qpu_cond_str[cond_add]);

		uint8_t sf = GET_BITFIELD(1, 45);

		if(sf)
		{
			printf(".sf");
		}

		uint8_t sem = GET_BITFIELD(0xf, 0);

		uint32_t imm_val = GET_BITFIELD(0x7ffffff, 5);

		printf("(%d, %#x) ; ", sem, imm_val);

		if(waddr_mul <= 31)
		{
			printf("rx%d", waddr_mul);
		}
		else
		{
			printf("%s", qpu_waddr_str[!ws][waddr_mul]);
		}

		if(pm)
		{
			printf(".%s", qpu_pack_mul_str[pack_mode]);
		}

		printf(" = %s", is_sem_inc ? "sem_inc" : "sem_dec");

		qpu_cond cond_mul = GET_BITFIELD(0x7, QPU_COND_MUL_SHIFT);

		printf(".%s() ;", qpu_cond_str[cond_mul]);
	}
	else if(!is_sem && sig_bits == QPU_SIG_LOAD_IMM)
	{
		qpu_load_type load_type = GET_BITFIELD(0x7f, 57) != 0x70;

		uint8_t is_signed = !GET_BITFIELD(1, 58);

		uint8_t pack_mode = GET_BITFIELD(0xf, QPU_PACK_SHIFT);

		if(!ws && !pm)
		{
			printf(".%s", qpu_pack_a_str[pack_mode]);
		}

		if(load_type == QPU_LOAD32)
		{
			printf(" = load32");
		}
		else
		{
			printf(" = load16");
		}

		if(ws)
		{
			printf(".ws");
		}

		if(pm)
		{
			printf(".pm");
		}

		qpu_cond cond_add = GET_BITFIELD(0x7, QPU_COND_ADD_SHIFT);

		printf(".%s", qpu_cond_str[cond_add]);

		uint8_t sf = GET_BITFIELD(1, 45);

		if(sf)
		{
			printf(".sf");
		}

		if(load_type == QPU_LOAD32)
		{
			uint32_t imm = GET_BITFIELD(0xffffffff, 0);

			printf("(%#x) ; ", imm);
		}
		else
		{
			if(is_signed)
			{
				printf(".signed");
			}

			uint16_t ms_imm = GET_BITFIELD(0xffff, 16);
			uint16_t ls_imm = GET_BITFIELD(0xffff, 0);
			printf(is_signed ? "(%#x, %#x) ; " : "(%#x, %#x) ; ", ms_imm, ls_imm);
		}

		if(waddr_mul <= 31)
		{
			printf("rx%d", waddr_mul);
		}
		else
		{
			printf("%s", qpu_waddr_str[!ws][waddr_mul]);
		}

		if(load_type == QPU_LOAD32)
		{
			printf(" = load32");
		}
		else
		{
			printf(" = load16");
		}

		if(pm)
		{
			printf(".%s", qpu_pack_mul_str[pack_mode]);
		}

		qpu_cond cond_mul = GET_BITFIELD(0x7, QPU_COND_MUL_SHIFT);

		printf(".%s() ;", qpu_cond_str[cond_mul]);
	}
	else if(!is_sem && sig_bits == QPU_SIG_BRANCH)
	{
		printf(" = branch");

		if(ws)
		{
			printf(".ws");
		}

		uint8_t is_relative = GET_BITFIELD(1, 51);

		if(is_relative)
		{
			printf(".rel");
		}

		uint8_t use_addr_a = GET_BITFIELD(1, 50);

		if(use_addr_a)
		{
			printf(".reg");
		}

		uint32_t imm = GET_BITFIELD(0xffffffff, 0);
		qpu_branch_cond branch_cond = GET_BITFIELD(0xf, QPU_BRANCH_COND_SHIFT);
		qpu_raddr raddr_a = GET_BITFIELD(0x1f, QPU_BRANCH_RADDR_A_SHIFT);

		printf("(%#x, %s, ", imm, qpu_branch_cond_str[branch_cond]);

		if(raddr_a <= 31)
		{
			if(raddr_a == 15)
			{
				printf("pay_zw");
			}
			else
			{
				printf("ra%d", raddr_a);
			}
		}
		else
		{
			printf("%s", qpu_raddr_str[0][raddr_a]);
		}

		printf(") ; ");

		if(waddr_mul <= 31)
		{
			printf("rx%d", waddr_mul);
		}
		else
		{
			printf("%s", qpu_waddr_str[!ws][waddr_mul]);
		}

		printf(" = branch() ;");
	}
	else
	{
		//ALU
		uint8_t pack_mode = GET_BITFIELD(0xf, QPU_PACK_SHIFT);

		if(!pm)
		{
			printf(".%s", qpu_pack_a_str[pack_mode]);
		}

		qpu_op_add op_add = GET_BITFIELD(0x1f, QPU_OP_ADD_SHIFT);

		printf(" = %s", qpu_op_add_str[op_add]);

		if(ws)
		{
			printf(".ws");
		}

		if(pm)
		{
			printf(".pm");
		}

		qpu_cond cond_add = GET_BITFIELD(0x7, QPU_COND_ADD_SHIFT);

		printf(".%s", qpu_cond_str[cond_add]);

		uint8_t sf = GET_BITFIELD(1, 45);

		if(sf)
		{
			printf(".sf");
		}

		qpu_unpack unpack_mode = GET_BITFIELD(0X7, QPU_UNPACK_SHIFT);

		printf(".%s", qpu_unpack_str[unpack_mode]);

		qpu_raddr raddr_a = GET_BITFIELD(0x3f, QPU_RADDR_A_SHIFT);
		qpu_raddr raddr_b = GET_BITFIELD(0x3f, QPU_RADDR_B_SHIFT);

		qpu_mux add_a = GET_BITFIELD(0x7, QPU_ADD_A_SHIFT);
		qpu_mux add_b = GET_BITFIELD(0x7, QPU_ADD_B_SHIFT);

		printf("(");

		printf("%s, %s, ", qpu_mux_str[add_a], qpu_mux_str[add_b]);

		if(raddr_a <= 31)
		{
			if(raddr_a == 15)
			{
				printf("pay_zw");
			}
			else
			{
				printf("ra%d", raddr_a);
			}
		}
		else
		{
			printf("%s", qpu_raddr_str[0][raddr_a]);
		}

		printf(", ");

		if(sig_bits == QPU_SIG_SMALL_IMM)
		{
			if(raddr_b < 16)
			{
				printf("%i", raddr_b);
			}
			else if(raddr_b < 32)
			{
				printf("%i", raddr_b - 32);
			}
			else
			{
				float val = raddr_b < 40 ? 1 << (raddr_b - 32) : 1.0f / (float)(1 << (48 - raddr_b));
				printf("%#x", *(uint32_t*)&val);
			}
		}
		else
		{
			if(raddr_b <= 31)
			{
				if(raddr_b == 15)
				{
					printf("pay_zw");
				}
				else
				{
					printf("rb%d", raddr_b);
				}
			}
			else
			{
				printf("%s", qpu_raddr_str[1][raddr_b]);
			}
		}

		printf(") ; ");

		if(waddr_mul <= 31)
		{
			printf("rx%d", waddr_mul);
		}
		else
		{
			printf("%s", qpu_waddr_str[!ws][waddr_mul]);
		}

		if(pm)
		{
			printf(".%s", qpu_pack_mul_str[pack_mode]);
		}

		qpu_op_mul op_mul = GET_BITFIELD(0x7, QPU_OP_MUL_SHIFT);

		printf(" = %s", qpu_op_mul_str[op_mul]);

		qpu_cond cond_mul = GET_BITFIELD(0x7, QPU_COND_MUL_SHIFT);

		printf(".%s", qpu_cond_str[cond_mul]);

		qpu_mux mul_a = GET_BITFIELD(0x7, QPU_MUL_A_SHIFT);
		qpu_mux mul_b = GET_BITFIELD(0x7, QPU_MUL_B_SHIFT);

		printf("(%s, %s) ; ", qpu_mux_str[mul_a], qpu_mux_str[mul_b]);
	}

	printf("\"\n");
}

unsigned get_num_instructions(char* ptr)
{
	unsigned num_instructions = 0;
	while(ptr && *ptr != '\0')
	{
		ptr = strstr(ptr, ";");
		if(!ptr) break;
		ptr = strstr(ptr+(ptr!=0), ";");
		if(!ptr) break;
		ptr = strstr(ptr+(ptr!=0), ";");
		if(ptr)
		{
			ptr += 1;
			num_instructions += 1;
		}
	}
	return num_instructions;
}
