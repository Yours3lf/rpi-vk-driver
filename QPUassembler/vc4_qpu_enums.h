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

#ifndef VC4_QPU_ENUMS_H
#define VC4_QPU_ENUMS_H

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
} qpu_mux;

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

//QPU pack MUL ALU values
typedef enum {
		QPU_PACK_MUL_NOP,
		QPU_PACK_MUL_8888 = 3, // converts mul float result to 8bit color in range [0...1] and replicate across all bytes of 32bit word
		QPU_PACK_MUL_8A, // converts mul float result to 8bit color in range [0...1]
		QPU_PACK_MUL_8B,
		QPU_PACK_MUL_8C,
		QPU_PACK_MUL_8D,
} qpu_pack_mul;

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

#endif /* VC4_QPU_ENUMS_H */
