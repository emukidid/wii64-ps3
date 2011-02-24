/**
 * Wii64 - fileBrowser-ps3.h
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


#ifndef FILE_BROWSER_ps3_H
#define FILE_BROWSER_ps3_H

extern fileBrowser_file topLevel_ps3_Default;  //PS3 USB
extern fileBrowser_file saveDir_ps3_Default;   //PS3 USB

int fileBrowser_ps3_readDir(fileBrowser_file*, fileBrowser_file**);
int fileBrowser_ps3_readFile(fileBrowser_file*, void*, unsigned int);
int fileBrowser_ps3_writeFile(fileBrowser_file*, void*, unsigned int);
int fileBrowser_ps3_seekFile(fileBrowser_file*, unsigned int, unsigned int);
int fileBrowser_ps3_init(fileBrowser_file* f);
int fileBrowser_ps3_deinit(fileBrowser_file* f);

int fileBrowser_ps3ROM_readFile(fileBrowser_file*, void*, unsigned int);
int fileBrowser_ps3ROM_deinit(fileBrowser_file* f);

#endif
