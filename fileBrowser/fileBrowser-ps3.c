/**
 * Wii64 - fileBrowser-ps3.c
 * Copyright (C) 2007, 2008, 2009 Mike Slegeir
 * Copyright (C) 2007, 2008, 2009 emu_kidid
 * 
 * fileBrowser for any devices using ps3
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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h> 
#include <fcntl.h> 
#include <sys/stat.h>
#include "fileBrowser.h"

fileBrowser_file topLevel_ps3_Default =
	{ "/dev_usb/wii64/roms", // file name
	  0, // offset
	  0, // size
	  FILE_BROWSER_ATTR_DIR
	 };
	 
fileBrowser_file saveDir_ps3_Default =
	{ "/dev_usb/wii64/saves",
	  0,
	  0,
	  FILE_BROWSER_ATTR_DIR
	 };
	 
 

extern void dbg_printf(const char *fmt,...);
int fileBrowser_ps3_readDir(fileBrowser_file* file, fileBrowser_file** dir){
  
  	struct dirent *dirent;
	DIR* dp = opendir( file->name );
	dbg_printf("Directory opened: %s\r\n",!dp ? "False":"True");
	if(!dp) return FILE_BROWSER_ERROR;
	struct stat fstat;
	
	// Set everything up to read
	int num_entries = 2, i = 0;
	*dir = malloc( num_entries * sizeof(fileBrowser_file) );
	// Read each entry of the directory
	while( (dirent = readdir(dp)) != 0 ){
		// Make sure we have room for this one
		if(i == num_entries){
			++num_entries;
			*dir = realloc( *dir, num_entries * sizeof(fileBrowser_file) ); 
		}
		sprintf((*dir)[i].name, "%s/%s", file->name, dirent->d_name);
		dbg_printf("Found: %s\r\n",(*dir)[i].name);
		stat((*dir)[i].name, &fstat);
		(*dir)[i].offset = 0;
		(*dir)[i].size   = fstat.st_size;
		(*dir)[i].attr   = (fstat.st_mode & S_IFDIR) ?
		                     FILE_BROWSER_ATTR_DIR : 0;
		++i;
	}
	
	closedir(dp);

	return num_entries;
}

int fileBrowser_ps3_seekFile(fileBrowser_file* file, unsigned int where, unsigned int type){
	if(type == FILE_BROWSER_SEEK_SET) file->offset = where;
	else if(type == FILE_BROWSER_SEEK_CUR) file->offset += where;
	else file->offset = file->size + where;
	
	return 0;
}

int fileBrowser_ps3_readFile(fileBrowser_file* file, void* buffer, unsigned int length){
	FILE* f = fopen( file->name, "rb" );
	if(!f) return FILE_BROWSER_ERROR;
	
	fseek(f, file->offset, SEEK_SET);
	int bytes_read = fread(buffer, 1, length, f);
	if(bytes_read > 0) file->offset += bytes_read;
	
	fclose(f);
	return bytes_read;
}

int fileBrowser_ps3_writeFile(fileBrowser_file* file, void* buffer, unsigned int length){
	FILE* f = fopen( file->name, "wb" );
	if(!f) return FILE_BROWSER_ERROR;
	
	fseek(f, file->offset, SEEK_SET);
	int bytes_read = fwrite(buffer, 1, length, f);
	if(bytes_read > 0) file->offset += bytes_read;
	
	fclose(f);
	return bytes_read;
}

/* call fileBrowser_ps3_init as much as you like for all devices
    - returns 0 on device not present/error
    - returns 1 on ok
*/
int fileBrowser_ps3_init(fileBrowser_file* f){
 	  return 1; //we're always ok
}

int fileBrowser_ps3_deinit(fileBrowser_file* f){
	return 0;
}


/* Special for ROM loading only */
static FILE* fd;

int fileBrowser_ps3ROM_deinit(fileBrowser_file* f){
	if(fd)
		fclose(fd);
	fd = NULL;
	
	return 0;
}
	
int fileBrowser_ps3ROM_readFile(fileBrowser_file* file, void* buffer, unsigned int length){

	if(!fd) fd = fopen( file->name, "rb");
	
	fseek(fd, file->offset, SEEK_SET);
	int bytes_read = fread(buffer, 1, length, fd);
	if(bytes_read > 0) file->offset += bytes_read;

	return bytes_read;
}

