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
#include <audio/audio.h>


#include "AudioPlugin.h"
#include "Audio_#1.1.h"

AUDIO_INFO AudioInfo;

#define NUM_BUFFERS 4
#define BUFFER_SIZE 3840
static char buffer[NUM_BUFFERS][BUFFER_SIZE] __attribute__((aligned(32)));
static int which_buffer = 0;
static unsigned int buffer_offset = 0;
#define NEXT(x) (x=(x+1)%NUM_BUFFERS)
static unsigned int freq;
static unsigned int real_freq;
static float freq_ratio;
extern void dbg_printf(const char *fmt,...);
// NOTE: 32khz actually uses ~2136 bytes/frame @ 60hz
static enum { BUFFER_SIZE_32_60 = 2112, BUFFER_SIZE_48_60 = 3200,
              BUFFER_SIZE_32_50 = 2560, BUFFER_SIZE_48_50 = 3840 } buffer_size;

#ifdef THREADED_AUDIO
static lwp_t audio_thread;
static sem_t buffer_full;
static sem_t buffer_empty;
static sem_t audio_free;
static sem_t first_audio;
static int   thread_running = 0;
#define AUDIO_STACK_SIZE 1024 // MEM: I could get away with a smaller stack
static char  audio_stack[AUDIO_STACK_SIZE];
#define AUDIO_PRIORITY 100
static int   thread_buffer = 0;
static int   audio_paused = 0;
#else // !THREADED_AUDIO
#define thread_buffer which_buffer
#endif

char audioEnabled;
u32 portNum;
audioPortParam params;
audioPortConfig config;
static u64 snd_key;
static sys_event_queue_t snd_queue;

EXPORT void CALL
AiDacrateChanged( int SystemType )
{
	// Taken from mupen_audio
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

	dbg_printf("Initializing frequency: %d (resampling ratio %f)\r\n",real_freq, freq_ratio);
	if( real_freq == 32000 ){
		//AUDIO_SetDSPSampleRate(AI_SAMPLERATE_32KHZ);
		buffer_size = (SystemType != SYSTEM_PAL) ?
		               BUFFER_SIZE_32_60 : BUFFER_SIZE_32_50;
	} else if( real_freq == 48000 ){
		//AUDIO_SetDSPSampleRate(AI_SAMPLERATE_48KHZ);
		buffer_size = (SystemType != SYSTEM_PAL) ?
		               BUFFER_SIZE_48_60 : BUFFER_SIZE_48_50;
	}
}

#ifdef THREADED_AUDIO
static void done_playing(void){
	// We're done playing, so we're releasing a buffer and the audio
	LWP_SemPost(buffer_empty);
	LWP_SemPost(audio_free);
}
#endif

void fillBuffer(f32 *buf)
{
	u32 i;
	static u32 pos = 0;

	for(i=0;i<AUDIO_BLOCK_SAMPLES;i++) {
		buf[i*2 + 0] = (f32)*((s16*)&buffer[thread_buffer][pos])/32768.0f;
		buf[i*2 + 1] = (f32)*((s16*)&buffer[thread_buffer][pos + 2])/32768.0f;
		
		pos += 4;
		if(pos>=buffer_size) pos = 0;
	}
}

void playOneBlock()
{
	f32 *buf;
	s32 ret = 0;
	sys_event_t event;
	u64 current_block = *(u64*)((u64)config.readIndex);
	f32 *dataStart = (f32*)((u64)config.audioDataStart);
	u32 audio_block_index = (current_block + 1)%config.numBlocks;

	ret = sysEventQueueReceive(snd_queue,&event,20*1000);

	buf = dataStart + config.channelCount*AUDIO_BLOCK_SAMPLES*audio_block_index;
	fillBuffer(buf);
}

static void inline play_buffer(void){
#ifndef THREADED_AUDIO
	// We should wait for the other buffer to finish its DMA transfer first
	//while( AUDIO_GetDMABytesLeft() );
	//AUDIO_StopDMA();

#else // THREADED_AUDIO
	// Wait for a sample to actually be played to work around a deadlock
	LWP_SemWait(first_audio);
	
	// This thread will keep giving buffers to the audio as they come
	while(thread_running){

	// Wait for a buffer to be processed
	LWP_SemWait(buffer_full);
#endif

	// Start the audio port
	int ret = audioPortStart(portNum);
	printf("audioPortStart: %08x\n",ret);
	
#ifdef THREADED_AUDIO
	// Wait for the audio interface to be free before playing
	LWP_SemWait(audio_free);
#endif

	playOneBlock();

#ifdef THREADED_AUDIO
	// Move the index to the next buffer
	NEXT(thread_buffer);
	}
#endif
}

static void inline copy_to_buffer(int* buffer, int* stream, unsigned int length){
	// NOTE: length is in samples (stereo (2) shorts)
	int di;
	float si;
	for(di = 0, si = 0.0f; di < length; ++di, si += freq_ratio){
#if 1
		// Linear interpolation between current and next sample
		float t = si - floorf(si);
		short* osample  = (short*)(buffer + di);
		short* isample1 = (short*)(stream + (int)si);
		short* isample2 = (short*)(stream + (int)ceilf(si));
		// Left and right
		osample[0] = (1.0f - t)*isample1[0] + t*isample2[0];
		osample[1] = (1.0f - t)*isample1[1] + t*isample2[1];
#else
		// Quick and dirty resampling: skip over or repeat samples
		buffer[di] = stream[(int)si];
#endif
	}
}

static void inline add_to_buffer(void* stream, unsigned int length){
	// This shouldn't lose any data and works for any size
	unsigned int stream_offset = 0;
	// Length calculations are in samples (stereo (short) sample pairs)
	unsigned int lengthi, rlengthi;
	unsigned int lengthLeft = length >> 2;
	unsigned int rlengthLeft = ceilf(lengthLeft / freq_ratio);
	while(1){
		rlengthi = (buffer_offset + (rlengthLeft << 2) <= buffer_size) ?
		            rlengthLeft : ((buffer_size - buffer_offset) >> 2);
		lengthi  = rlengthi * freq_ratio;

#ifdef THREADED_AUDIO
		// Wait for a buffer we can copy into
		LWP_SemWait(buffer_empty);
#endif
		copy_to_buffer(buffer[which_buffer] + buffer_offset,
		               stream + stream_offset, rlengthi);

		if(buffer_offset + (rlengthLeft << 2) < buffer_size){
			buffer_offset += rlengthi << 2;
#ifdef THREADED_AUDIO
			// This is a little weird, but we didn't fill this buffer.
			//   So it is still considered 'empty', but since we 'decremented'
			//   buffer_empty coming in here, we want to set it back how
			//   it was, so we don't cause a deadlock
			LWP_SemPost(buffer_empty);
#endif
			return;
		}

		lengthLeft    -= lengthi;
		stream_offset += lengthi << 2;
		rlengthLeft   -= rlengthi;

#ifdef THREADED_AUDIO
		// Start the thread if this is the first sample
		if(!thread_running){
			thread_running = 1;
			LWP_SemPost(first_audio);
		}
		
		// Let the audio thread know that we've filled a new buffer
		LWP_SemPost(buffer_full);
#else
		play_buffer();
#endif

		NEXT(which_buffer);
		buffer_offset = 0;
	}
}

EXPORT void CALL
AiLenChanged( void )
{
	if(!audioEnabled) return;

	short* stream = (short*)(AudioInfo.RDRAM +
		         (*AudioInfo.AI_DRAM_ADDR_REG & 0xFFFFFF));
	unsigned int length = *AudioInfo.AI_LEN_REG;

	add_to_buffer(stream, length);
}

EXPORT DWORD CALL
AiReadLength( void )
{
	// I don't know if this is the data they're trying to get
	return 0;
	//return AUDIO_GetDMABytesLeft();
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
	
	s32 ret = audioInit();

	dbg_printf("audioInit: %08x\n",ret);

	params.numChannels = AUDIO_PORT_2CH;
	params.numBlocks = AUDIO_BLOCK_8;
	params.attrib = 0x1000;
	params.level = 1.0f;
	ret = audioPortOpen(&params,&portNum);
	dbg_printf("audioPortOpen: %08x\n",ret);
	dbg_printf("      portNum: %d\n",portNum);

	ret = audioGetPortConfig(portNum,&config);
	dbg_printf("audioGetPortConfig: %08x\n",ret);
	dbg_printf("config.readIndex: %08x\n",config.readIndex);
	dbg_printf("config.status: %d\n",config.status);
	dbg_printf("config.channelCount: %ld\n",config.channelCount);
	dbg_printf("config.numBlocks: %ld\n",config.numBlocks);
	dbg_printf("config.portSize: %d\n",config.portSize);
	dbg_printf("config.audioDataStart: %08x\n",config.audioDataStart);

	ret = audioCreateNotifyEventQueue(&snd_queue,&snd_key);
	dbg_printf("audioCreateNotifyEventQueue: %08x\n",ret);
	dbg_printf("snd_queue: %16lx\n",(long unsigned int)snd_queue);
	dbg_printf("snd_key: %16lx\n",snd_key);

	ret = audioSetNotifyEventQueue(snd_key);
	dbg_printf("audioSetNotifyEventQueue: %08x\n",ret);

	ret = sysEventQueueDrain(snd_queue);
	dbg_printf("sysEventQueueDrain: %08x\n",ret);
	return TRUE;
}

EXPORT void CALL RomOpen()
{
#ifdef THREADED_AUDIO
	// Create our semaphores and start/resume the audio thread; reset the buffer index
	LWP_SemInit(&buffer_full, 0, NUM_BUFFERS);
	LWP_SemInit(&buffer_empty, NUM_BUFFERS, NUM_BUFFERS);
	LWP_SemInit(&audio_free, 0, 1);
	LWP_SemInit(&first_audio, 0, 1);
	thread_running = 0;
	LWP_CreateThread(&audio_thread, (void*)play_buffer, NULL, audio_stack, AUDIO_STACK_SIZE, AUDIO_PRIORITY);
	AUDIO_RegisterDMACallback(done_playing);
	thread_buffer = which_buffer = 0;
	audio_paused = 1;
#endif
}

EXPORT void CALL
RomClosed( void )
{
#ifdef THREADED_AUDIO
	// Destroy semaphores and suspend the thread so audio can't play
	if(!thread_running) LWP_SemPost(first_audio);
	thread_running = 0;
	LWP_SemDestroy(buffer_full);
	LWP_SemDestroy(buffer_empty);
	LWP_SemDestroy(audio_free);
	LWP_SemDestroy(first_audio);
	LWP_JoinThread(audio_thread, NULL);
	audio_paused = 0;
#endif
	// So we don't have a buzzing sound when we exit the game
	int ret = audioPortStop(portNum);
	dbg_printf("audioPortStop: %08x\n",ret);
}

EXPORT void CALL
ProcessAlist( void )
{
}

void pauseAudio(void){
#ifdef THREADED_AUDIO
	// We just grab the audio_free 'lock' and don't let go
	//   when we have this lock, audio_thread must be waiting
	if(audioEnabled){
		LWP_SemWait(audio_free);
		audio_paused = 1;
	}
#endif
	int ret = audioPortStop(portNum);
	dbg_printf("audioPortStop: %08x\n",ret);
}

void resumeAudio(void){
#ifdef THREADED_AUDIO
	if(audio_paused && audioEnabled){
		// When we're want the audio to resume, release the 'lock'
		LWP_SemPost(audio_free);
		audio_paused = 0;
	}
#endif
}

