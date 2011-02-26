/**
 * Wii64 - ROM-Cache-MEM2.c (Wii ROM Cache)
 * Copyright (C) 2007, 2008, 2009 Mike Slegeir
 * Copyright (C) 2007, 2008, 2009 emu_kidid
 * 
 * This is how the ROM should be accessed, this way the ROM doesn't waste RAM
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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <ppu-types.h>
#include "winlnxdefs.h"
#include "../fileBrowser/fileBrowser.h"
//#include "../gui/gui_GX-menu.h"
#include "../r4300/r4300.h"
//#include "../gui/DEBUG.h"
//#include "../gui/GUI.h"
#include "ROM-Cache.h"

#define MAX_ROM_SIZE 64*1024*1024

//#ifdef MENU_V2
//void LoadingBar_showBar(float percent, const char* string);
//#define PRINT DUMMY_print
//#define SETLOADPROG DUMMY_setLoadProg
//#define DRAWGUI DUMMY_draw
//#else
//#define PRINT GUI_print
//#define SETLOADPROG GUI_setLoadProg
//#define DRAWGUI GUI_draw
//#endif

static u32   ROMSize;
//static int   ROMCompressed;
//static int   ROMHeaderSize;

//extern void showLoadProgress(float);
//extern void pauseAudio(void);
//extern void resumeAudio(void);
extern BOOL hasLoadedROM;

//void DUMMY_print(char* string) { }
//void DUMMY_setLoadProg(float percent) { }
//void DUMMY_draw() { }

//static void ensure_block(u32 block);

//PKZIPHEADER pkzip;

// The ROM
u8 *rom = NULL;
int readBefore = 0;
fileBrowser_file *ROMFile;

void ROMCache_init(fileBrowser_file* f){
	if(!rom) {
		rom = malloc(MAX_ROM_SIZE);
	}
	readBefore = 0; //de-init byteswapping
	ROMFile = f;
	ROMSize = f->size;

	romFile_seekFile(f, 0, FILE_BROWSER_SEEK_SET);	// Lets be nice and keep the file at 0.
}

void ROMCache_deinit() {}

void* ROMCache_pointer(u32 rom_offset){
	return &rom[rom_offset];
}

void ROMCache_read(u8* dest, u32 offset, u32 length){
	memcpy(dest, &rom[offset], length);
}

int ROMCache_load(fileBrowser_file* f){
	//PRINT("Loading ROM %s into RAM fully");

	romFile_seekFile(ROMFile, 0, FILE_BROWSER_SEEK_SET);
	
	u32 offset = 0,loads_til_update = 0;
	int bytes_read;
	while(offset < ROMSize){
		bytes_read = romFile_readFile(ROMFile, rom + offset, 262144);
		
		if(bytes_read < 0){		// Read fail!
			//SETLOADPROG( -1.0f );
			return -1;
		}
		//initialize byteswapping if it isn't already
		if(!readBefore)
		{
 			if(init_byte_swap(*(unsigned int*)rom) == BYTE_SWAP_BAD) {
 			  romFile_deinit(ROMFile);
 			  return -2;
		  }
 			readBefore = 1;
		}
		//byteswap
		byte_swap(&rom[offset], bytes_read);
		
		offset += bytes_read;
		
		if(!loads_til_update--){
			//SETLOADPROG( (float)offset/ROMSize );
			//DRAWGUI();
			//LoadingBar_showBar((float)offset/ROMSize, txt);
			loads_til_update = 16;
		}
	}
	
	//SETLOADPROG( -1.0f );
	return 0;
}



