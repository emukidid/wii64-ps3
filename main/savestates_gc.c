/**
 * Mupen64 - savestates.c
 * Copyright (C) 2002 Hacktarux
 * Copyright (C) 2008, 2009 emu_kidid
 *
 * Mupen64 homepage: http://mupen64.emulation64.com
 * email address: hacktarux@yahoo.fr
 *                emukidid@gmail.com
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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "savestates.h"
#include "guifuncs.h"
#include "rom.h"
#include "../gc_memory/memory.h"
#include "../gc_memory/flashram.h"
#include "../r4300/macros.h"
#include "../r4300/r4300.h"
#include "../r4300/interupt.h"
#include "wii64config.h"

extern int *autoinc_save_slot;
void pauseAudio(void);
void resumeAudio(void);

int savestates_job = 0;

static unsigned int savestates_slot = 0;

void savestates_select_slot(unsigned int s)
{
   if (s > 9) return;
   savestates_slot = s;
}
	
//returns 0 on file not existing
int savestates_exists(int mode)
{
  
  return 1;
}

void savestates_save()
{ 
  
}

void savestates_load()
{
	
}
