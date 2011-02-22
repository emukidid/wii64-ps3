/**
 * Mupen64 - r4300.h
 * Copyright (C) 2002 Hacktarux
 *
 * Mupen64 homepage: http://mupen64.emulation64.com
 * email address: hacktarux@yahoo.fr
 * 
 * If you want to contribute to the project please contact
 * me first (maybe someone is already making what you are
 * planning to do).
 *
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
 * You should have received a copy of the GNU General Public
 * Licence along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
**/

#ifndef R4300_H
#define R4300_H

#include <stdio.h>
#include <string.h>
#include <ppu-types.h>
#include "../main/rom.h"
#include "../gc_memory/tlb.h"
#include "recomp.h"

#define LOW_WORD(reg) (*((s32*)&(reg)+1))

typedef struct {
	u32 pc, last_pc;
	long long int gpr[32];
	long long int hi, lo;
	long long int local_gpr[2];
	u32 reg_cop0[32];
	long long int fpr_data[32];
	double*       fpr_double[32];
	float*        fpr_single[32];
	s32          fcr0, fcr31;
	u32  next_interrupt, cic_chip;
	u32 delay_slot, skip_jump;
	s32           stop, llbit;
	tlb*          tlb_e; // [32]
} R4300;
extern R4300 r4300;

#define local_rs (r4300.local_gpr[0])
#define local_rt (r4300.local_gpr[1])
#define local_rs32 LOW_WORD(local_rs)
#define local_rt32 LOW_WORD(local_rt)

extern precomp_instr *PC;

extern u32 jump_target;
extern tlb tlb_e[32];
extern u32 dyna_interp;
extern unsigned long long int debug_count;
extern u32 dynacore;
extern u32 interpcore;
extern s32 rounding_mode, trunc_mode, round_mode, ceil_mode, floor_mode;
//extern char invalid_code[0x100000];
extern u32 jump_to_address;
extern s32 no_audio_delay;
extern s32 no_compiled_jump;

void cpu_init(void);
void cpu_deinit(void);
void go();
void pure_interpreter();
void compare_core();
inline void jump_to_func();
void update_count();
int check_cop1_unusable();
void shuffle_fpr_data(int oldStatus, int newStatus);
void set_fpr_pointers(int newStatus);

#define jump_to(a) { jump_to_address = a; jump_to_func(); }

// profiling

#define GFX_SECTION 1
#define AUDIO_SECTION 2
#define COMPILER_SECTION 3
#define IDLE_SECTION 4
#define TLB_SECTION 5
#define FP_SECTION 6
#define INTERP_SECTION 7
#define TRAMP_SECTION 8
#define FUNCS_SECTION 9
#define LINK_SECTION 10
#define UNLINK_SECTION 11
#define DYNAMEM_SECTION 12
#define NUM_SECTIONS 12

//#define PROFILE

#ifdef PROFILE

void start_section(int section_type);
void end_section(int section_type);
void refresh_stat();

#else

#define start_section(a)
#define end_section(a)
#define refresh_stat()

#endif

#endif

