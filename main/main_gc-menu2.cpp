/**
 * Wii64 - main_gc-menu2.cpp (aka MenuV2)
 * Copyright (C) 2007, 2008, 2009, 2010 Mike Slegeir
 * Copyright (C) 2007, 2008, 2009, 2010 sepp256
 * Copyright (C) 2007, 2008, 2009, 2010 emu_kidid
 * 
 * New main that uses menu's instead of prompts
 *
 * Wii64 homepage: http://www.emulatemii.com
 * email address: tehpola@gmail.com
 *                sepp256@gmail.com
 *                emukidid@gmail.com
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
**/


/* INCLUDES */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include "timers.h"

#include "winlnxdefs.h"
extern "C" {
#include "main.h"
#include "rom.h"
#include "plugin.h"
#include "../gc_input/controller.h"

#include "../r4300/r4300.h"
#include "../gc_memory/memory.h"
#include "../gc_memory/tlb.h"
#include "../gc_memory/pif.h"
#include "../gc_memory/flashram.h"
#include "../gc_memory/Saves.h"
#include "../main/savestates.h"
#include "wii64config.h"
}

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <ppu-types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/socket.h>
int s;
struct sockaddr_in server;
#define TESTIP				"192.168.1.100"
#define TESTPORT			18194

void udp_setup()
{
	s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	memset(&server, 0, sizeof(server));
	server.sin_len = sizeof(server);
	server.sin_family = AF_INET;
	inet_pton(AF_INET, TESTIP, &server.sin_addr);
	server.sin_port = htons(TESTPORT);
}

extern "C" {
void dbg_printf(const char *fmt,...)
{
	int len;
	va_list args;
	char str[1024];

	va_start(args, fmt);
	len = vsprintf(str,fmt,args);
	va_end(args);
	
	sendto(s, str, len, 0, (struct sockaddr*)&server, sizeof(server));
}
}

static void Initialise (void){
	udp_setup();
	int i = 0;
	dbg_printf("Initialized wii64 on PS3!!!!!!!!!!\r\n");
}


/* NECESSARY FUNCTIONS AND VARIABLES */

// -- Initialization functions --
static void Initialise(void);
static void gfx_info_init(void);
static void audio_info_init(void);
static void rsp_info_init(void);
void control_info_init(void);
// -- End init functions --

// -- Plugin data --
//#define DEFAULT_FIFO_SIZE    (256*1024)//(64*1024) minimum

CONTROL Controls[4];

static GFX_INFO     gfx_info;
       AUDIO_INFO   audio_info;
static CONTROL_INFO control_info;
static RSP_INFO     rsp_info;

extern timers Timers;

extern "C" void gfx_set_fb(unsigned int* fb1, unsigned int* fb2);
void gfx_set_window(int x, int y, int width, int height);
// -- End plugin data --

static u32* xfb[2] = { NULL, NULL };	/*** Framebuffers ***/
int GX_xfb_offset = 0;

// Dummy functions
static void dummy_func(){ }
void (*fBRead)(DWORD addr) = NULL;
void (*fBWrite)(DWORD addr, DWORD size) = NULL;
void (*fBGetFrameBufferInfo)(void *p) = NULL;
void new_frame(){ }
void new_vi(){ }
int loadROM();

int main(int argc, char* argv[]){
	/* INITIALIZE */
Initialise();
	loadROM();
	go();
	return 0;
}

int loadROM(){
  int ret = 0;
  rom_read();
  dbg_printf("format_mempacks()\r\n");
  format_mempacks();
  dbg_printf("reset_flashram()\r\n");
  reset_flashram();
  dbg_printf("init_eeprom()\r\n");
  init_eeprom();
	// Init everything for this ROM
	dbg_printf("init_memory()\r\n");
	init_memory();
	dbg_printf("gfx_set_fb()\r\n");
	//gfx_set_fb(xfb[0], xfb[1]);
	dbg_printf("gfx_info_init()\r\n");
	gfx_info_init();
	dbg_printf("audio_info_init()\r\n");
	audio_info_init();
	dbg_printf("control_info_init()\r\n");
	control_info_init();
	dbg_printf("rsp_info_init()\r\n");
	rsp_info_init();

	dbg_printf("romOpen_gfx()\r\n");
	romOpen_gfx();
	
//	gfx_set_fb(xfb[0], xfb[1]);
	dbg_printf("romOpen_audio()\r\n");
	romOpen_audio();
	dbg_printf("romOpen_input()\r\n");
	romOpen_input();
	
	dbg_printf("cpu_init()\r\n");
	cpu_init();
	dbg_printf("end of cpu_init()\r\n");
	return 0;
}

static void gfx_info_init(void){
	gfx_info.MemoryBswaped = TRUE;
	gfx_info.HEADER = (BYTE*)ROM_HEADER;
	gfx_info.RDRAM = (BYTE*)rdram;
	gfx_info.DMEM = (BYTE*)SP_DMEM;
	gfx_info.IMEM = (BYTE*)SP_IMEM;
	gfx_info.MI_INTR_REG = &(MI_register.mi_intr_reg);
	gfx_info.DPC_START_REG = &(dpc_register.dpc_start);
	gfx_info.DPC_END_REG = &(dpc_register.dpc_end);
	gfx_info.DPC_CURRENT_REG = &(dpc_register.dpc_current);
	gfx_info.DPC_STATUS_REG = &(dpc_register.dpc_status);
	gfx_info.DPC_CLOCK_REG = &(dpc_register.dpc_clock);
	gfx_info.DPC_BUFBUSY_REG = &(dpc_register.dpc_bufbusy);
	gfx_info.DPC_PIPEBUSY_REG = &(dpc_register.dpc_pipebusy);
	gfx_info.DPC_TMEM_REG = &(dpc_register.dpc_tmem);
	gfx_info.VI_STATUS_REG = &(vi_register.vi_status);
	gfx_info.VI_ORIGIN_REG = &(vi_register.vi_origin);
	gfx_info.VI_WIDTH_REG = &(vi_register.vi_width);
	gfx_info.VI_INTR_REG = &(vi_register.vi_v_intr);
	gfx_info.VI_V_CURRENT_LINE_REG = &(vi_register.vi_current);
	gfx_info.VI_TIMING_REG = &(vi_register.vi_burst);
	gfx_info.VI_V_SYNC_REG = &(vi_register.vi_v_sync);
	gfx_info.VI_H_SYNC_REG = &(vi_register.vi_h_sync);
	gfx_info.VI_LEAP_REG = &(vi_register.vi_leap);
	gfx_info.VI_H_START_REG = &(vi_register.vi_h_start);
	gfx_info.VI_V_START_REG = &(vi_register.vi_v_start);
	gfx_info.VI_V_BURST_REG = &(vi_register.vi_v_burst);
	gfx_info.VI_X_SCALE_REG = &(vi_register.vi_x_scale);
	gfx_info.VI_Y_SCALE_REG = &(vi_register.vi_y_scale);
	gfx_info.CheckInterrupts = dummy_func;
	initiateGFX(gfx_info);
}

static void audio_info_init(void){
	audio_info.MemoryBswaped = TRUE;
	audio_info.HEADER = (BYTE*)ROM_HEADER;
	audio_info.RDRAM = (BYTE*)rdram;
	audio_info.DMEM = (BYTE*)SP_DMEM;
	audio_info.IMEM = (BYTE*)SP_IMEM;
	audio_info.MI_INTR_REG = &(MI_register.mi_intr_reg);
	audio_info.AI_DRAM_ADDR_REG = &(ai_register.ai_dram_addr);
	audio_info.AI_LEN_REG = &(ai_register.ai_len);
	audio_info.AI_CONTROL_REG = &(ai_register.ai_control);
	audio_info.AI_STATUS_REG = &(ai_register.ai_status); // FIXME: This was set to dummy
	audio_info.AI_DACRATE_REG = &(ai_register.ai_dacrate);
	audio_info.AI_BITRATE_REG = &(ai_register.ai_bitrate);
	audio_info.CheckInterrupts = dummy_func;
	initiateAudio(audio_info);
}

void control_info_init(void){
	control_info.MemoryBswaped = TRUE;
	control_info.HEADER = (BYTE*)ROM_HEADER;
	control_info.Controls = Controls;
	int i;
	for (i=0; i<4; i++)
	  {
	     Controls[i].Present = FALSE;
	     Controls[i].RawData = FALSE;
	     Controls[i].Plugin = PLUGIN_NONE;
	  }
	initiateControllers(control_info);
}

static void rsp_info_init(void){
	static int cycle_count;
	rsp_info.MemoryBswaped = TRUE;
	rsp_info.RDRAM = (BYTE*)rdram;
	rsp_info.DMEM = (BYTE*)SP_DMEM;
	rsp_info.IMEM = (BYTE*)SP_IMEM;
	rsp_info.MI_INTR_REG = &MI_register.mi_intr_reg;
	rsp_info.SP_MEM_ADDR_REG = &sp_register.sp_mem_addr_reg;
	rsp_info.SP_DRAM_ADDR_REG = &sp_register.sp_dram_addr_reg;
	rsp_info.SP_RD_LEN_REG = &sp_register.sp_rd_len_reg;
	rsp_info.SP_WR_LEN_REG = &sp_register.sp_wr_len_reg;
	rsp_info.SP_STATUS_REG = &sp_register.sp_status_reg;
	rsp_info.SP_DMA_FULL_REG = &sp_register.sp_dma_full_reg;
	rsp_info.SP_DMA_BUSY_REG = &sp_register.sp_dma_busy_reg;
	rsp_info.SP_PC_REG = &rsp_register.rsp_pc;
	rsp_info.SP_SEMAPHORE_REG = &sp_register.sp_semaphore_reg;
	rsp_info.DPC_START_REG = &dpc_register.dpc_start;
	rsp_info.DPC_END_REG = &dpc_register.dpc_end;
	rsp_info.DPC_CURRENT_REG = &dpc_register.dpc_current;
	rsp_info.DPC_STATUS_REG = &dpc_register.dpc_status;
	rsp_info.DPC_CLOCK_REG = &dpc_register.dpc_clock;
	rsp_info.DPC_BUFBUSY_REG = &dpc_register.dpc_bufbusy;
	rsp_info.DPC_PIPEBUSY_REG = &dpc_register.dpc_pipebusy;
	rsp_info.DPC_TMEM_REG = &dpc_register.dpc_tmem;
	rsp_info.CheckInterrupts = dummy_func;
	rsp_info.ProcessDlistList = processDList;
	rsp_info.ProcessAlistList = processAList;
	rsp_info.ProcessRdpList = processRDPList;
	rsp_info.ShowCFB = showCFB;
	initiateRSP(rsp_info,(DWORD*)&cycle_count);
}

void stop_it() { r4300.stop = 1; }

