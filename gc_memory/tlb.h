/**
 * Mupen64 - tlb.h
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

#ifndef TLB_H
#define TLB_H
#include <ppu-types.h>

typedef struct _tlb
{
   short mask;
   s32 vpn2;
   char g;
   unsigned char asid;
   s32 pfn_even;
   char c_even;
   char d_even;
   char v_even;
   s32 pfn_odd;
   char c_odd;
   char d_odd;
   char v_odd;
   char r;
   //long check_parity_mask;
   
   u32 start_even;
   u32 end_even;
   u32 phys_even;
   u32 start_odd;
   u32 end_odd;
   u32 phys_odd;
} tlb;

extern u32 tlb_LUT_r[0x100000];
extern u32 tlb_LUT_w[0x100000];
void tlb_mem2_init();
u32 virtual_to_physical_address(u32 addresse, int w);
int probe_nop(u32 address);

#endif
