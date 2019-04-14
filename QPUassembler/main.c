#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
					qpu_op_mul op_mul,
					qpu_op_add op_add,
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
							  qpu_op_mul op_mul,
							  qpu_op_add op_add,
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
					  op_mul,
					  op_add,
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

void parse_dst(char** str, qpu_waddr* waddr, uint8_t* pack_mode, uint8_t* ws, unsigned is_add)
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

	if(!waddr_res && dst && dst[0] == 'r')
	{
		unsigned is_a = dst[1] == 'a' ? 1 : 0;

		//add normally writes to regfile A
		*ws = !is_add && is_a;

		waddr_res = strtol(dst+2, 0, 0);
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
	*pack_mode = pack_mode_res;
}

void parse_op_modifiers(char** str, uint8_t* signed_or_unsigned, uint8_t* pm, uint8_t* sf, qpu_cond* condition, qpu_unpack* unpack_mode, uint8_t* rel, uint8_t* reg)
{
	char* modifier = strtok(*str, ".");

	//at most 4 modifiers supported
	for(int c = 0; c < 4; ++c)
	{
		if(modifier)
		{
			*str = modifier;

			if(strcmp(modifier, "pm") == 0)
			{
				*pm = 1;
				modifier = strtok(0, ".");
				continue;
			}

			if(strcmp(modifier, "rel") == 0)
			{
				*rel = 1;
				modifier = strtok(0, ".");
				continue;
			}

			if(strcmp(modifier, "reg") == 0)
			{
				*reg = 1;
				modifier = strtok(0, ".");
				continue;
			}

			if(strcmp(modifier, "sf") == 0)
			{
				*sf = 1;
				modifier = strtok(0, ".");
				continue;
			}

			if(strcmp(modifier, "signed") == 0)
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

			unsigned num_unpack_modes = sizeof(qpu_unpack_str) / sizeof(const char *);

			for(unsigned d = 0; d < num_unpack_modes; ++d)
			{
				if(qpu_unpack_str[d] && strcmp(modifier, qpu_unpack_str[d]) == 0)
				{
					*unpack_mode = d;
					break;
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

void parse_op(char** str, qpu_alu_type* type, qpu_op_add* op_add, qpu_op_mul* op_mul, uint8_t* is_sem_inc, qpu_load_type* load_type)
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

		for(unsigned c = 0; c < num_add_ops && op; ++c)
		{
			if(qpu_op_add_str[c] && strcmp(op, qpu_op_add_str[c]) == 0)
			{
				*op_add = c;
				break;
			}
		}

		for(unsigned c = 0; c < num_mul_ops && op; ++c)
		{
			if(qpu_op_mul_str[c] && strcmp(op, qpu_op_mul_str[c]) == 0)
			{
				*op_mul = c;
				break;
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

void parse_args_alu(char** str, qpu_mux* in_a, qpu_mux* in_b, uint8_t* small_imm, uint8_t* raddr_a, uint8_t* raddr_b)
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
		uint32_t si = strtol(arg, 0, 0);
		*small_imm = qpu_encode_small_immediate(si);
		*str = arg;
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
			raddr_a_res = strtol(arg+2, 0, 0);
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
			raddr_b_res = strtol(arg+2, 0, 0);
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
		*sem = strtol(arg, 0, 0);
		*str = arg;
	}

	arg = strtok(0, " \n\v\f\r\t,");

	if(arg)
	{
		*imm32 = strtol(arg, 0, 0);
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
		*imm32 = strtol(arg, 0, 0);
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
			raddr_a_res = strtol(arg+2, 0, 0);
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
			*imm32 = strtol(arg, 0, 0);
			*str = arg;
		}
	}
	else
	{
		if(arg)
		{
			*ms_imm16 = strtol(arg, 0, 0);
			*str = arg;
		}

		arg = strtok(0, " \n\v\f\r\t,");

		if(arg)
		{
			*ls_imm16 = strtol(arg, 0, 0);
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
		qpu_cond cond_mul = QPU_COND_ALWAYS;
		qpu_cond cond_add = QPU_COND_ALWAYS;
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
		parse_dst(&token, &waddr_add, &pack_mode, &ws, 1);

		//check op
		token = strtok(token, " \n\v\f\r\t.=(");
		parse_op(&token, &type, &op_add, &op_mul, &is_sem_inc, &load_type);

		//get modifiers
		token = strtok(token, " \n\v\f\r\t(");
		parse_op_modifiers(&token, &is_signed, &pack_unpack_select, &sf, &cond_add, &unpack_mode, &rel, &reg);

		if(type == QPU_ALU)
		{
			//get arguments for add
			token = strtok(token, ")");
			parse_args_alu(&token, &add_a, &add_b, &imm32, &raddr_a, &raddr_b);
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
		parse_dst(&token, &waddr_mul, &pack_mode, &ws, 0);

		//check op
		token = strtok(token, " \n\v\f\r\t.=(");
		parse_op(&token, &type, &op_add, &op_mul, &is_sem_inc, &load_type);

		//get modifiers
		token = strtok(token, " \n\v\f\r\t(");
		parse_op_modifiers(&token, &is_signed, &pack_unpack_select, &sf, &cond_mul, &unpack_mode, &rel, &reg);

		token = strtok(token, ")");

		if(type == QPU_ALU)
		{
			//get arguments for mul
			parse_args_alu(&token, &mul_a, &mul_b, &imm32, &raddr_a, &raddr_b);
		}

		//EMIT INSTRUCTION HERE
		if(type == QPU_ALU)
		{
			if(sig_bit == QPU_SIG_SMALL_IMM)
			{
				instructions[instruction_counter] = encode_alu_small_imm(unpack_mode, pack_unpack_select, pack_mode, cond_add, cond_mul, sf, ws, waddr_add, waddr_mul, op_mul, op_add, raddr_a, imm32, add_a, add_b, mul_a, mul_b);
			}
			else
			{
				instructions[instruction_counter] = encode_alu(sig_bit, unpack_mode, pack_unpack_select, pack_mode, cond_add, cond_mul, sf, ws, waddr_add, waddr_mul, op_mul, op_add, raddr_a, raddr_b, add_a, add_b, mul_a, mul_b);
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

}

/*
Format:
#comment
sig_bit_optional	; dstAdd.pack_mode_optional	= add_opcode.pm_optional.sf_optional.condition.unpack_mode_optional(srcA, srcB, imm_optional, raddr_a_optional, raddr_b_optional)	; dstMul.pack_mode_optional = mul_opcode.condition(srcA, srcB)	;
sig_bit_branch		; dstAdd					= branch.pm_optional.rel_optional.reg_optional(address, condition, srcA_optional)													; dstMul					= branch()							;
sig_bit_none		; dstAdd.pack_mode_optional	= sem_inc.pm_optional.sf_optional.condition(sem_number, 27bit_imm_value_optional)													; dstMul.pack_mode_optional = sem_inc.condition()				;
sig_load_imm		; dstAdd.pack_mode_optional	= load32.pm_optional.sf_optional.condition(immediate_value)																			; dstMul.pack_mode_optional = load32.condition()				;
sig_load_imm		; dstAdd.pack_mode_optional	= load16.pm_optional.signed_optional.sf_optional.condition(int16_imm, in16_imm)														; dstMul.pack_mode_optional = load16.condition()				;

Examples:
sig_none			; ra0.nop					= add.pm.sf.always(r0, r1, 0)																										; rb0.nop					= fmul.sf.always(r2, r3)			;
sig_branch			; ra0						= branch.pm.rel.reg.always(0xdeadbeef, ra1)																							; rb0						= branch()							;
sig_none			; ra0.nop					= sem_inc.pm.sf.always(1, 0x7ffffff)																								; rb0.nop					= sem_inc.always()					;
sig_load_imm		; ra0.nop					= load32.pm.sf.always(0xdeadbeef)																									; rb0.nop					= load32.always()					;
sig_load_imm		; ra0.nop					= load16.pm.sf.signed.always(1, 2)																									; rb0.nop					= load16.always()					;
#mov
sig_none			; ra0.nop					= or(r0, r0)																														; rb0						= v8min(r1, r1)						;
 */

int main()
{
	char asm_code[] =
			"sig_none		; ra0.nop	= add.sf.always.nop(r0, r1, 0)				; rb0.nop	= fmul.sf.always(r2, r3)	;"
			"sig_branch		; ra0		= branch.rel.reg(0xdeadbeef, always, ra1)	; rb0		= branch()					;"
			"#hello\n"
			"sig_none		; ra0.nop	= sem_inc.sf.always(1, 0x7ffffff)			; rb0.nop	= sem_inc.always()			;"
			"#hello2\n"
			"sig_load_imm	; ra0.nop	= load32.sf.always(0xdeadbeef)				; rb0.nop	= load32.always()			;"
			"sig_load_imm	; ra0.nop	= load16.sf.signed.always(0xdead, 0xbeef)	; rb0.nop	= load16.always()			;";

	unsigned num_instructions = 0;
	char* ptr = asm_code;
	while(ptr && *ptr != '\0')
	{
		ptr = strstr(ptr, ";");
		ptr = strstr(ptr+(ptr!=0), ";");
		ptr = strstr(ptr+(ptr!=0), ";");
		if(ptr)
		{
			ptr += 1;
			num_instructions += 1;
		}
	}

	printf("Num instructions: %i\n", num_instructions);

	if(!num_instructions)
	{
		return 0;
	}

	uint64_t* instruction_size = sizeof(uint64_t)*num_instructions;
	uint64_t* instructions = malloc(instruction_size);

	assemble_qpu_asm(asm_code, instructions);

	for(int c = 0; c < instruction_size; ++c)
	{
		unsigned char d = ((char*)instructions)[c];
		printf("%#x,\t", d);
		if((c+1)%8==0)
		{
			printf("\n");
		}
	}

	const char asm_instructions[] =
	{
		0x53,	0x70,	0x9e,	0x2c,	0,	0x60,	0x2,	0x10,
		0xef,	0xbe,	0xad,	0xde,	0,	0x20,	0xfc,	0xf0,
		0xf1,	0xff,	0xff,	0xff,	0,	0x60,	0x2,	0xe8,
		0xef,	0xbe,	0xad,	0xde,	0,	0x60,	0x2,	0xe0,
		0xef,	0xbe,	0xad,	0xde,	0,	0x60,	0x2,	0xe2
	};

	for(int c = 0; c < num_instructions; ++c)
	{
		disassemble_qpu_asm(asm_instructions);
	}

	return 0;
}
