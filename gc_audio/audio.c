/**
 * Wii64 - audio.c
 * Copyright (C) 2007, 2008, 2009 Mike Slegeir
 * Copyright (C) 2007, 2008, 2009 emu_kidid
 * 
 * Low-level Audio plugin with linear interpolation & 
 * resampling to 32/48KHz for the GC/Wii
 *
 * Wii64 homepage: http://www.emulatemii.com
 * email address: tehpola@gmail.com
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

/*  MEMORY USAGE:
     STATIC:
   	Audio Buffer: 4x BUFFER_SIZE (currently ~3kb each)
   	LWP Control Buffer: 1Kb
*/

#include "../main/winlnxdefs.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "AudioPlugin.h"
#include "Audio_#1.1.h"

static unsigned int freq;
static unsigned int real_freq;
static float freq_ratio;
AUDIO_INFO AudioInfo;
extern void dbg_printf(const char *fmt,...);
char audioEnabled;

EXPORT void CALL
AiDacrateChanged( int SystemType ) {
	freq = 32000; //default to 32khz incase we get a bad systemtype
	switch (SystemType){
	      case SYSTEM_NTSC:
		freq = 48681812 / (*AudioInfo.AI_DACRATE_REG + 1);
		break;
	      case SYSTEM_PAL:
		freq = 49656530 / (*AudioInfo.AI_DACRATE_REG + 1);
		break;
	      case SYSTEM_MPAL:
		freq = 48628316 / (*AudioInfo.AI_DACRATE_REG + 1);
		break;
	}

	// Calculate the absolute differences from 32 and 48khz
	int diff32 = freq - 32000;
	int diff48 = freq - 48000;
	diff32 = diff32 > 0 ? diff32 : -diff32;
	diff48 = diff48 > 0 ? diff48 : -diff48;
	// Choose the closest real frequency
	real_freq = (diff32 < diff48) ? 32000 : 48000;
	freq_ratio = (float)freq / real_freq;

	dbg_printf("Initializing frequency: %d (resampling ratio %f)\r\n",
	        real_freq, freq_ratio);
}

unsigned int length = 0;
EXPORT void CALL
AiLenChanged( void )
{
	short* stream = (short*)(AudioInfo.RDRAM +
		         (*AudioInfo.AI_DRAM_ADDR_REG & 0xFFFFFF));
	length = *AudioInfo.AI_LEN_REG;
	//dbg_printf("AiLenChanged. stream: %04X, length %08X\r\n",stream,length);
}

EXPORT DWORD CALL
AiReadLength( void )
{
	return 0;
}

EXPORT void CALL
AiUpdate( BOOL Wait )
{
}

EXPORT void CALL
CloseDLL( void )
{
}

EXPORT void CALL
DllAbout( HWND hParent )
{
	printf ("Gamecube audio plugin\n\tby Mike Slegeir" );
}

EXPORT void CALL
DllConfig ( HWND hParent )
{
}

EXPORT void CALL
DllTest ( HWND hParent )
{
}

EXPORT void CALL
GetDllInfo( PLUGIN_INFO * PluginInfo )
{
	PluginInfo->Version = 0x0101;
	PluginInfo->Type    = PLUGIN_TYPE_AUDIO;
	sprintf(PluginInfo->Name,"Gamecube audio plugin\n\tby Mike Slegeir");
	PluginInfo->NormalMemory  = TRUE;
	PluginInfo->MemoryBswaped = TRUE;
}

EXPORT BOOL CALL
InitiateAudio( AUDIO_INFO Audio_Info )
{
	AudioInfo = Audio_Info;
	return TRUE;
}

EXPORT void CALL RomOpen()
{

}

EXPORT void CALL
RomClosed( void )
{

}

EXPORT void CALL
ProcessAlist( void )
{
}

void pauseAudio(void){

}

void resumeAudio(void){

}

