/*
 * Copyright © 2014 Broadcom
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef VC4_QPU_DEFINES_H
#define VC4_QPU_DEFINES_H

#include <assert.h>
#include <stdint.h>

typedef enum{
	QPU_ALU,
	QPU_SEM,
	QPU_BRANCH,
	QPU_LOAD_IMM
} qpu_alu_type;

typedef enum{
	QPU_LOAD32,
	QPU_LOAD16
} qpu_load_type;

//Condition Codes
//The QPU keeps a set of N, Z and C flag bits per 16 SIMD element. These flags are updated based on the result
//of the ADD ALU if the ‘sf’ bit is set. If the sf bit is set and the ADD ALU executes a NOP or its condition code was
//NEVER, flags are set based upon the result of the MUL ALU result.
typedef enum {
		QPU_COND_NEVER,
		QPU_COND_ALWAYS,
		QPU_COND_ZS, //set
		QPU_COND_ZC, //clear
		QPU_COND_NS,
		QPU_COND_NC,
		QPU_COND_CS,
		QPU_COND_CC,
} qpu_cond;

static const char *qpu_cond_str[] = {
		[QPU_COND_NEVER] = "never",
		[QPU_COND_ALWAYS] = "always",
		[QPU_COND_ZS] = "zs",
		[QPU_COND_ZC] = "zc",
		[QPU_COND_NS] = "ns",
		[QPU_COND_NC] = "nc",
		[QPU_COND_CS] = "cs",
		[QPU_COND_CC] = "cc",
};

//ALU Input muxes
//selects one register for input
//The add_a, add_b, mul_a, and mul_b fields specify the input data for the A and B ports of the ADD and MUL
//pipelines, respectively
typedef enum {
		/* hardware mux values */
		QPU_MUX_R0,
		QPU_MUX_R1,
		QPU_MUX_R2,
		QPU_MUX_R3,
		QPU_MUX_R4, //special purpose, read only
		QPU_MUX_R5, //special purpose
		QPU_MUX_A,
		QPU_MUX_B,

		/**
		 * Non-hardware mux value, stores a small immediate field to be
		 * programmed into raddr_b in the qpu_reg.index.
		 */
		QPU_MUX_SMALL_IMM,
} qpu_mux;

static const char *qpu_mux_str[] = {
		[QPU_MUX_R0] = "r0",
		[QPU_MUX_R1] = "r1",
		[QPU_MUX_R2] = "r2",
		[QPU_MUX_R3] = "r3",
		[QPU_MUX_R4] = "r4",
		[QPU_MUX_R5] = "r5",
		[QPU_MUX_A] = "a",
		[QPU_MUX_B] = "b",
		[QPU_MUX_SMALL_IMM] = "imm",
};


//Signaling Bits
//The 4-bit signaling field signal is connected to the 3d pipeline and is set to indicate one of a number of
//conditions to the 3d hardware. Values from this field are also used to encode a ‘BKPT’ instruction, and to
//encode Branches and Load Immediate instructions.
typedef enum {
		QPU_SIG_SW_BREAKPOINT,
		QPU_SIG_NONE,
		QPU_SIG_THREAD_SWITCH,
		QPU_SIG_PROG_END,
		QPU_SIG_WAIT_FOR_SCOREBOARD, //stall until this QPU can safely access tile buffer
		QPU_SIG_SCOREBOARD_UNLOCK,
		QPU_SIG_LAST_THREAD_SWITCH,
		QPU_SIG_COVERAGE_LOAD, //from tile buffer to r4
		QPU_SIG_COLOR_LOAD, //from tile buffer to r4
		QPU_SIG_COLOR_LOAD_END, //color load and program end
		QPU_SIG_LOAD_TMU0, //read data from TMU0 to r4
		QPU_SIG_LOAD_TMU1, //read data from TMU1 to r4
		QPU_SIG_ALPHA_MASK_LOAD, //from tile buffer to r4
		QPU_SIG_SMALL_IMM, //ALU instruction with raddr_b specifying small immediate or vector rotate
		QPU_SIG_LOAD_IMM, //load immediate instruction
		QPU_SIG_BRANCH
} qpu_sig_bits;

static const char *qpu_sig_bits_str[] = {
		[QPU_SIG_SW_BREAKPOINT] = "sig_brk",
		[QPU_SIG_NONE] = "sig_none",
		[QPU_SIG_THREAD_SWITCH] = "sig_switch",
		[QPU_SIG_PROG_END] = "sig_end",
		[QPU_SIG_WAIT_FOR_SCOREBOARD] = "sig_wait_score",
		[QPU_SIG_SCOREBOARD_UNLOCK] = "sig_unlock_score",
		[QPU_SIG_LAST_THREAD_SWITCH] = "sig_thread_switch",
		[QPU_SIG_COVERAGE_LOAD] = "sig_coverage_load",
		[QPU_SIG_COLOR_LOAD] = "sig_color_load",
		[QPU_SIG_COLOR_LOAD_END] = "sig_color_load_end",
		[QPU_SIG_LOAD_TMU0] = "sig_load_tmu0",
		[QPU_SIG_LOAD_TMU1] = "sig_load_tmu1",
		[QPU_SIG_ALPHA_MASK_LOAD] = "sig_alpha_mask_load",
		[QPU_SIG_SMALL_IMM] = "sig_small_imm",
		[QPU_SIG_LOAD_IMM] = "sig_load_imm",
		[QPU_SIG_BRANCH] = "sig_branch",
};

//Small immediate encoding
//Returns the small immediate value to be encoded in to the raddr b field if
//the argument can be represented as one, or ~0 otherwise.
//48: Small immediate value for rotate-by-r5, and 49-63 are "rotate by n channels"
uint8_t qpu_encode_small_immediate(uint32_t i)
{
		if (i <= 15)
				return i;
		if ((int)i < 0 && (int)i >= -16)
				return i + 32;

		switch (i) {
		case 0x3f800000:
				return 32;
		case 0x40000000:
				return 33;
		case 0x40800000:
				return 34;
		case 0x41000000:
				return 35;
		case 0x41800000:
				return 36;
		case 0x42000000:
				return 37;
		case 0x42800000:
				return 38;
		case 0x43000000:
				return 39;
		case 0x3b800000:
				return 40;
		case 0x3c000000:
				return 41;
		case 0x3c800000:
				return 42;
		case 0x3d000000:
				return 43;
		case 0x3d800000:
				return 44;
		case 0x3e000000:
				return 45;
		case 0x3e800000:
				return 46;
		case 0x3f000000:
				return 47;
		}

		return ~0;
}

//QPU unpack values
//(can be used to unpack from r4 too)
typedef enum {
		QPU_UNPACK_NOP,
		QPU_UNPACK_16A, //from A reg: convert 16bit float to 32bit float, or 16bit int to 32bit int, depending on the instruction
		QPU_UNPACK_16B,
		QPU_UNPACK_8D_REP, //replicate most significant byte (alpha) across word: {a, a, a, a}
		QPU_UNPACK_8A, //convert 8bit color in range [0...1] to 32bit float or 32bit int, depending on the instruction
		QPU_UNPACK_8B,
		QPU_UNPACK_8C,
		QPU_UNPACK_8D,
} qpu_unpack;

static const char *qpu_unpack_str[] = {
		[QPU_UNPACK_NOP] = "nop",
		[QPU_UNPACK_16A] = "16a",
		[QPU_UNPACK_16B] = "16b",
		[QPU_UNPACK_8D_REP] = "8d_rep",
		[QPU_UNPACK_8A] = "8a",
		[QPU_UNPACK_8B] = "8b",
		[QPU_UNPACK_8C] = "8c",
		[QPU_UNPACK_8D] = "8d",
};

//QPU pack regfile A
typedef enum {
		QPU_PACK_A_NOP,
		QPU_PACK_A_16A, //convert to 16 bit float if float input, or to int16 (just takes least significant 16bits)
		QPU_PACK_A_16B,
		QPU_PACK_A_8888, //convert to 8bit uint (just takes least significant 8bits) and replicate across all bytes of 32bit word
		QPU_PACK_A_8A, // Convert to 8-bit unsigned int. (just takes least significant 8bits)
		QPU_PACK_A_8B,
		QPU_PACK_A_8C,
		QPU_PACK_A_8D,

		// Saturating variants of the previous instructions.
		QPU_PACK_A_32_SAT, //saturate signed 32bit number (takes into account overflow/carry flags)
		QPU_PACK_A_16A_SAT, //convert to 16bit float if float input, or int16, depending on input (with saturation)
		QPU_PACK_A_16B_SAT,
		QPU_PACK_A_8888_SAT, //convert to uint8 with saturation and replicate across all bytes of 32bit word
		QPU_PACK_A_8A_SAT, //conver to uint8 with saturation
		QPU_PACK_A_8B_SAT,
		QPU_PACK_A_8C_SAT,
		QPU_PACK_A_8D_SAT,
} qpu_pack_a;

static const char *qpu_pack_a_str[] = {
		[QPU_PACK_A_NOP] = "nop",
		[QPU_PACK_A_16A] = "16a",
		[QPU_PACK_A_16B] = "16b",
		[QPU_PACK_A_8888] = "8888",
		[QPU_PACK_A_8A] = "8a",
		[QPU_PACK_A_8B] = "8b",
		[QPU_PACK_A_8C] = "8c",
		[QPU_PACK_A_8D] = "8d",

		[QPU_PACK_A_32_SAT] = "sat",
		[QPU_PACK_A_16A_SAT] = "16a.sat",
		[QPU_PACK_A_16B_SAT] = "16b.sat",
		[QPU_PACK_A_8888_SAT] = "8888.sat",
		[QPU_PACK_A_8A_SAT] = "8a.sat",
		[QPU_PACK_A_8B_SAT] = "8b.sat",
		[QPU_PACK_A_8C_SAT] = "8c.sat",
		[QPU_PACK_A_8D_SAT] = "8d.sat",
};

//QPU pack MUL ALU values
typedef enum {
		QPU_PACK_MUL_NOP,
		QPU_PACK_MUL_8888 = 3, // converts mul float result to 8bit color in range [0...1] and replicate across all bytes of 32bit word
		QPU_PACK_MUL_8A, // converts mul float result to 8bit color in range [0...1]
		QPU_PACK_MUL_8B,
		QPU_PACK_MUL_8C,
		QPU_PACK_MUL_8D,
} qpu_pack_mul;

static const char *qpu_pack_mul_str[] = {
		[QPU_PACK_MUL_NOP] = "nop",
		[QPU_PACK_MUL_8888] = "8888",
		[QPU_PACK_MUL_8A] = "8a",
		[QPU_PACK_MUL_8B] = "8b",
		[QPU_PACK_MUL_8C] = "8c",
		[QPU_PACK_MUL_8D] = "8d",
};

typedef enum {
		QPU_COND_BRANCH_ALL_ZS, //all z flags set
		QPU_COND_BRANCH_ALL_ZC, //all z flags clear
		QPU_COND_BRANCH_ANY_ZS,
		QPU_COND_BRANCH_ANY_ZC,
		QPU_COND_BRANCH_ALL_NS,
		QPU_COND_BRANCH_ALL_NC,
		QPU_COND_BRANCH_ANY_NS,
		QPU_COND_BRANCH_ANY_NC,
		QPU_COND_BRANCH_ALL_CS,
		QPU_COND_BRANCH_ALL_CC,
		QPU_COND_BRANCH_ANY_CS,
		QPU_COND_BRANCH_ANY_CC,

		QPU_COND_BRANCH_ALWAYS = 15 //always execute
} qpu_branch_cond;

static const char *qpu_branch_cond_str[] = {
		[QPU_COND_BRANCH_ALL_ZS] = "all_zs",
		[QPU_COND_BRANCH_ALL_ZC] = "all_zc",
		[QPU_COND_BRANCH_ANY_ZS] = "any_zs",
		[QPU_COND_BRANCH_ANY_ZC] = "any_zc",
		[QPU_COND_BRANCH_ALL_NS] = "all_ns",
		[QPU_COND_BRANCH_ALL_NC] = "all_nc",
		[QPU_COND_BRANCH_ANY_NS] = "any_ns",
		[QPU_COND_BRANCH_ANY_NC] = "any_nc",
		[QPU_COND_BRANCH_ALL_CS] = "all_cs",
		[QPU_COND_BRANCH_ALL_CC] = "all_cc",
		[QPU_COND_BRANCH_ANY_CS] = "any_cs",
		[QPU_COND_BRANCH_ANY_CC] = "any_cc",
		[QPU_COND_BRANCH_ALWAYS] = "always",
};

//QPU ADD instruction set
typedef enum {
        QPU_A_NOP,
		QPU_A_FADD, //float add
        QPU_A_FSUB,
        QPU_A_FMIN,
        QPU_A_FMAX,
		QPU_A_FMINABS, //float min(abs(x))
        QPU_A_FMAXABS,
		QPU_A_FTOI, //convert float to int
		QPU_A_ITOF, //convert int to float
		QPU_A_ADD = 12, //int add
        QPU_A_SUB,
		QPU_A_SHR, //int shift right
		QPU_A_ASR, //int arithmetic shift right
		QPU_A_ROR, //int rotate right
		QPU_A_SHL, //int shift left
		QPU_A_MIN,
        QPU_A_MAX,
        QPU_A_AND,
        QPU_A_OR,
        QPU_A_XOR,
        QPU_A_NOT,
		QPU_A_CLZ, //int count leading zeroes
		QPU_A_V8ADDS = 30, //add with saturation per 8bit element
		QPU_A_V8SUBS = 31,
} qpu_op_add;

static const char *qpu_op_add_str[] = {
		[QPU_A_NOP] = "nop",
		[QPU_A_FADD] = "fadd",
		[QPU_A_FSUB] = "fsub",
		[QPU_A_FMIN] = "fmin",
		[QPU_A_FMAX] = "fmax",
		[QPU_A_FMINABS] = "fminabs",
		[QPU_A_FMAXABS] = "fmaxabs",
		[QPU_A_FTOI] = "ftoi",
		[QPU_A_ITOF] = "itof",
		[QPU_A_ADD] = "add",
		[QPU_A_SUB] = "sub",
		[QPU_A_SHR] = "shr",
		[QPU_A_ASR] = "asr",
		[QPU_A_ROR] = "ror",
		[QPU_A_SHL] = "shl",
		[QPU_A_MIN] = "min",
		[QPU_A_MAX] = "max",
		[QPU_A_AND] = "and",
		[QPU_A_OR] = "or",
		[QPU_A_XOR] = "xor",
		[QPU_A_NOT] = "not",
		[QPU_A_CLZ] = "clz",
		[QPU_A_V8ADDS] = "v8adds",
		[QPU_A_V8SUBS] = "v8subs",
};

//QPU MUL instruction set
typedef enum {
        QPU_M_NOP,
		QPU_M_FMUL, //float mul
		QPU_M_MUL24, //24bit int mul?
		QPU_M_V8MULD, //mul two vectors of 8bit ints in range [0...1]
        QPU_M_V8MIN,
        QPU_M_V8MAX,
		QPU_M_V8ADDS, //add two vectors of 8bit ints in range [0...1] with saturation
        QPU_M_V8SUBS,
} qpu_op_mul;

static const char *qpu_op_mul_str[] = {
		[QPU_M_NOP] = "nop",
		[QPU_M_FMUL] = "fmul",
		[QPU_M_MUL24] = "mul24",
		[QPU_M_V8MULD] = "v8muld",
		[QPU_M_V8MIN] = "v8min",
		[QPU_M_V8MAX] = "v8max",
		[QPU_M_V8ADDS] = "v8adds",
		[QPU_M_V8SUBS] = "v8subs",
};

//read and write ops may mean different things...
//hence two maps

//QPU register address read map
typedef enum {
        QPU_R_FRAG_PAYLOAD_ZW = 15, /* W for A file, Z for B file */
        /* 0-31 are the plain regfile a or b fields */
		QPU_R_UNIF = 32, //uniform read
		QPU_R_VARY = 35, //varying read
		QPU_R_ELEM_QPU = 38, //element number
        QPU_R_NOP,
		QPU_R_XY_PIXEL_COORD = 41, // X for regfile a, Y for regfile b
		QPU_R_MS_FLAGS = 42, //A reg
		QPU_R_REV_FLAG = 42, //B reg
        QPU_R_VPM = 48,
		QPU_R_VPM_LD_BUSY = 49, //load busy for reg A
		QPU_R_VPM_ST_BUSY = 49, //store busy for reg B
		QPU_R_VPM_LD_WAIT = 50, //load wait for reg A
		QPU_R_VPM_ST_WAIT = 50, //store wait for reg B
        QPU_R_MUTEX_ACQUIRE,
} qpu_raddr;

static const char *qpu_raddr_str[][52] = {
	{ //A
		[QPU_R_FRAG_PAYLOAD_ZW] = "pay_zw",
		[QPU_R_UNIF] = "uni",
		[QPU_R_VARY] = "vary",
		[QPU_R_ELEM_QPU] = "elem",
		[QPU_R_NOP] = "nop",
		[QPU_R_XY_PIXEL_COORD] = "x_pix",
		[QPU_R_MS_FLAGS] = "ms_flags",
		[QPU_R_VPM] = "vpm_read",
		[QPU_R_VPM_LD_BUSY] = "vpm_ld_busy",
		[QPU_R_VPM_LD_WAIT] = "vpm_ld_wait",
		[QPU_R_MUTEX_ACQUIRE] = "mutex_acq"
	},
	{ //B
	  [QPU_R_FRAG_PAYLOAD_ZW] = "pay_zw",
	  [QPU_R_UNIF] = "uni",
	  [QPU_R_VARY] = "vary",
	  [QPU_R_ELEM_QPU] = "elem",
	  [QPU_R_NOP] = "nop",
	  [QPU_R_XY_PIXEL_COORD] = "y_pix",
	  [QPU_R_REV_FLAG] = "rev_flag",
	  [QPU_R_VPM] = "vpm_read",
	  [QPU_R_VPM_ST_BUSY] = "vpm_st_busy",
	  [QPU_R_VPM_ST_WAIT] = "vpm_st_wait",
	  [QPU_R_MUTEX_ACQUIRE] = "mutex_acq"
	}
};

//QPU register address write map
typedef enum {
        /* 0-31 are the plain regfile a or b fields */
		QPU_W_ACC0 = 32, //accumulation 0, aka r0
        QPU_W_ACC1,
        QPU_W_ACC2,
        QPU_W_ACC3,
        QPU_W_TMU_NOSWAP,
		QPU_W_ACC5, //replicate pixel0 per quad for reg A, replicate SIMD element0 for reg B
		QPU_W_HOST_INT, //host interrupt
        QPU_W_NOP,
        QPU_W_UNIFORMS_ADDRESS,
		QPU_W_QUAD_XY, // X for regfile a, Y for regfile b
		QPU_W_MS_FLAGS = 42, //A reg
		QPU_W_REV_FLAG = 42, //B reg
        QPU_W_TLB_STENCIL_SETUP = 43,
        QPU_W_TLB_Z,
        QPU_W_TLB_COLOR_MS,
        QPU_W_TLB_COLOR_ALL,
        QPU_W_TLB_ALPHA_MASK,
        QPU_W_VPM,
        QPU_W_VPMVCD_SETUP, /* LD for regfile a, ST for regfile b */
        QPU_W_VPM_ADDR, /* LD for regfile a, ST for regfile b */
        QPU_W_MUTEX_RELEASE,
		QPU_W_SFU_RECIP, //special function unit 1/x
		QPU_W_SFU_RECIPSQRT,  //1/sqrt(x)
        QPU_W_SFU_EXP,
        QPU_W_SFU_LOG,
        QPU_W_TMU0_S,
        QPU_W_TMU0_T,
        QPU_W_TMU0_R,
        QPU_W_TMU0_B,
        QPU_W_TMU1_S,
        QPU_W_TMU1_T,
        QPU_W_TMU1_R,
        QPU_W_TMU1_B,
} qpu_waddr;

static const char *qpu_waddr_str[][64] = {
	{ //A
		[QPU_W_ACC0] = "r0",
		[QPU_W_ACC1] = "r1",
		[QPU_W_ACC2] = "r2",
		[QPU_W_ACC3] = "r3",
		[QPU_W_TMU_NOSWAP] = "tmu_noswap",
		[QPU_W_ACC5] = "r5",
		[QPU_W_HOST_INT] = "host_int",
		[QPU_W_NOP] = "nop",
		[QPU_W_UNIFORMS_ADDRESS] = "uniforms_addr",
		[QPU_W_QUAD_XY] = "quad_x",
		[QPU_W_MS_FLAGS] = "ms_flags",
		[QPU_W_TLB_STENCIL_SETUP] = "tlb_stencil_setup",
		[QPU_W_TLB_Z] = "tlb_z",
		[QPU_W_TLB_COLOR_MS] = "tlb_color_ms",
		[QPU_W_TLB_COLOR_ALL] = "tlb_color_all",
		[QPU_W_VPM] = "vpm",
		[QPU_W_VPMVCD_SETUP] = "vr_setup",
		[QPU_W_VPM_ADDR] = "vr_addr",
		[QPU_W_MUTEX_RELEASE] = "mutex_release",
		[QPU_W_SFU_RECIP] = "sfu_recip",
		[QPU_W_SFU_RECIPSQRT] = "sfu_recipsqrt",
		[QPU_W_SFU_EXP] = "sfu_exp",
		[QPU_W_SFU_LOG] = "sfu_log",
		[QPU_W_TMU0_S] = "tmu0_s",
		[QPU_W_TMU0_T] = "tmu0_t",
		[QPU_W_TMU0_R] = "tmu0_r",
		[QPU_W_TMU0_B] = "tmu0_b",
		[QPU_W_TMU1_S] = "tmu1_s",
		[QPU_W_TMU1_T] = "tmu1_t",
		[QPU_W_TMU1_R] = "tmu1_r",
		[QPU_W_TMU1_B] = "tmu1_b",
	},
	{ //B
	  [QPU_W_ACC0] = "r0",
	  [QPU_W_ACC1] = "r1",
	  [QPU_W_ACC2] = "r2",
	  [QPU_W_ACC3] = "r3",
	  [QPU_W_TMU_NOSWAP] = "tmu_noswap",
	  [QPU_W_ACC5] = "r5",
	  [QPU_W_HOST_INT] = "host_int",
	  [QPU_W_NOP] = "nop",
	  [QPU_W_UNIFORMS_ADDRESS] = "uniforms_addr",
	  [QPU_W_QUAD_XY] = "quad_y",
	  [QPU_W_REV_FLAG] = "rev_flags",
	  [QPU_W_TLB_STENCIL_SETUP] = "tlb_stencil_setup",
	  [QPU_W_TLB_Z] = "tlb_z",
	  [QPU_W_TLB_COLOR_MS] = "tlb_color_ms",
	  [QPU_W_TLB_COLOR_ALL] = "tlb_color_all",
	  [QPU_W_VPM] = "vpm",
	  [QPU_W_VPMVCD_SETUP] = "vw_setup",
	  [QPU_W_VPM_ADDR] = "vw_addr",
	  [QPU_W_MUTEX_RELEASE] = "mutex_release",
	  [QPU_W_SFU_RECIP] = "sfu_recip",
	  [QPU_W_SFU_RECIPSQRT] = "sfu_recipsqrt",
	  [QPU_W_SFU_EXP] = "sfu_exp",
	  [QPU_W_SFU_LOG] = "sfu_log",
	  [QPU_W_TMU0_S] = "tmu0_s",
	  [QPU_W_TMU0_T] = "tmu0_t",
	  [QPU_W_TMU0_R] = "tmu0_r",
	  [QPU_W_TMU0_B] = "tmu0_b",
	  [QPU_W_TMU1_S] = "tmu1_s",
	  [QPU_W_TMU1_T] = "tmu1_t",
	  [QPU_W_TMU1_R] = "tmu1_r",
	  [QPU_W_TMU1_B] = "tmu1_b",
	}
};

#define QPU_MASK(high, low) ((((uint64_t)1<<((high)-(low)+1))-1)<<(low))
/* Using the GNU statement expression extension */
#define QPU_SET_FIELD(value, field)                                       \
        ({                                                                \
                uint64_t fieldval = (uint64_t)(value) << field ## _SHIFT; \
                assert((fieldval & ~ field ## _MASK) == 0);               \
                fieldval & field ## _MASK;                                \
         })

#define QPU_GET_FIELD(word, field) ((uint32_t)(((word)  & field ## _MASK) >> field ## _SHIFT))

#define QPU_UPDATE_FIELD(inst, value, field)                              \
        (((inst) & ~(field ## _MASK)) | QPU_SET_FIELD(value, field))

#define QPU_SIG_SHIFT                   60
#define QPU_SIG_MASK                    QPU_MASK(63, 60)

#define QPU_UNPACK_SHIFT                57
#define QPU_UNPACK_MASK                 QPU_MASK(59, 57)

#define QPU_LOAD_IMM_MODE_SHIFT         57
#define QPU_LOAD_IMM_MODE_MASK          QPU_MASK(59, 57)
# define QPU_LOAD_IMM_MODE_U32          0
# define QPU_LOAD_IMM_MODE_I2           1
# define QPU_LOAD_IMM_MODE_U2           3

/**
 * If set, the pack field means PACK_MUL or R4 packing, instead of normal
 * regfile a packing.
 */
#define QPU_PM                          ((uint64_t)1 << 56)

#define QPU_PACK_SHIFT                  52
#define QPU_PACK_MASK                   QPU_MASK(55, 52)

#define QPU_COND_ADD_SHIFT              49
#define QPU_COND_ADD_MASK               QPU_MASK(51, 49)
#define QPU_COND_MUL_SHIFT              46
#define QPU_COND_MUL_MASK               QPU_MASK(48, 46)


#define QPU_BRANCH_COND_SHIFT           52
#define QPU_BRANCH_COND_MASK            QPU_MASK(55, 52)

#define QPU_BRANCH_REL                  ((uint64_t)1 << 51)
#define QPU_BRANCH_REG                  ((uint64_t)1 << 50)

#define QPU_BRANCH_RADDR_A_SHIFT        45
#define QPU_BRANCH_RADDR_A_MASK         QPU_MASK(49, 45)

#define QPU_SF                          ((uint64_t)1 << 45)

#define QPU_WADDR_ADD_SHIFT             38
#define QPU_WADDR_ADD_MASK              QPU_MASK(43, 38)
#define QPU_WADDR_MUL_SHIFT             32
#define QPU_WADDR_MUL_MASK              QPU_MASK(37, 32)

#define QPU_OP_MUL_SHIFT                29
#define QPU_OP_MUL_MASK                 QPU_MASK(31, 29)

#define QPU_RADDR_A_SHIFT               18
#define QPU_RADDR_A_MASK                QPU_MASK(23, 18)
#define QPU_RADDR_B_SHIFT               12
#define QPU_RADDR_B_MASK                QPU_MASK(17, 12)
#define QPU_SMALL_IMM_SHIFT             12
#define QPU_SMALL_IMM_MASK              QPU_MASK(17, 12)
/* Small immediate value for rotate-by-r5, and 49-63 are "rotate by n
 * channels"
 */
#define QPU_SMALL_IMM_MUL_ROT		48

#define QPU_ADD_A_SHIFT                 9
#define QPU_ADD_A_MASK                  QPU_MASK(11, 9)
#define QPU_ADD_B_SHIFT                 6
#define QPU_ADD_B_MASK                  QPU_MASK(8, 6)
#define QPU_MUL_A_SHIFT                 3
#define QPU_MUL_A_MASK                  QPU_MASK(5, 3)
#define QPU_MUL_B_SHIFT                 0
#define QPU_MUL_B_MASK                  QPU_MASK(2, 0)

#define QPU_WS                          ((uint64_t)1 << 44)

#define QPU_OP_ADD_SHIFT                24
#define QPU_OP_ADD_MASK                 QPU_MASK(28, 24)

#define QPU_LOAD_IMM_SHIFT              0
#define QPU_LOAD_IMM_MASK               QPU_MASK(31, 0)

#define QPU_BRANCH_TARGET_SHIFT         0
#define QPU_BRANCH_TARGET_MASK          QPU_MASK(31, 0)

#endif /* VC4_QPU_DEFINES_H */
