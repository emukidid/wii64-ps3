/**
 * glN64_GX - N64.h
 * Copyright (C) 2003 Orkin
 *
 * glN64 homepage: http://gln64.emulation64.com
 * Wii64 homepage: http://www.emulatemii.com
 *
**/

#ifndef N64_H
#define N64_H

#include "Types.h"

#define MI_INTR_DP		0x20		// Bit 5: DP intr 

struct N64Regs
{
	unsigned int *MI_INTR;

	unsigned int *DPC_START;
	unsigned int *DPC_END;
	unsigned int *DPC_CURRENT;
	unsigned int *DPC_STATUS;
	unsigned int *DPC_CLOCK;
	unsigned int *DPC_BUFBUSY;
	unsigned int *DPC_PIPEBUSY;
	unsigned int *DPC_TMEM;

	unsigned int *VI_STATUS;
	unsigned int *VI_ORIGIN;
	unsigned int *VI_WIDTH;
	unsigned int *VI_INTR;
	unsigned int *VI_V_CURRENT_LINE;
	unsigned int *VI_TIMING;
	unsigned int *VI_V_SYNC;
	unsigned int *VI_H_SYNC;
	unsigned int *VI_LEAP;
	unsigned int *VI_H_START;
	unsigned int *VI_V_START;
	unsigned int *VI_V_BURST;
	unsigned int *VI_X_SCALE;
	unsigned int *VI_Y_SCALE;
};

extern N64Regs REG;
extern u8 *DMEM;
extern u8 *IMEM;
extern u8 *RDRAM;
extern u64 TMEM[512];
extern u32 RDRAMSize;

#endif

