/**
 * glN64_GX - VI.h
 * Copyright (C) 2003 Orkin
 * Copyright (C) 2008, 2009 sepp256 (Port to Wii/Gamecube/PS3)
 *
 * glN64 homepage: http://gln64.emulation64.com
 * Wii64 homepage: http://www.emulatemii.com
 * email address: sepp256@gmail.com
 *
**/

#ifndef VI_H
#define VI_H
#include "Types.h"

struct VIInfo
{
	u32 width, height;
	u32 lastOrigin;
#if (defined(__GX__)||defined(PS3))
	unsigned int* xfb[2];
	int which_fb;
	bool updateOSD;
	bool enableLoadIcon;
	bool EFBcleared;
	bool copy_fb;
#endif // __GX__ PS3
};

extern VIInfo VI;

void VI_UpdateSize();
void VI_UpdateScreen();

#ifdef PS3
void VI_RSX_showFPS();
void VI_RSX_showDEBUG();
void VI_GX_updateDEBUG();
#endif // PS3

#ifdef __GX__

void VI_GX_init();
void VI_GX_setFB(unsigned int* fb1, unsigned int* fb2);
unsigned int* VI_GX_getScreenPointer();
void VI_GX_clearEFB();
void VI_GX_showFPS();
void VI_GX_showLoadProg(float percent);
void VI_GX_updateDEBUG();
void VI_GX_showDEBUG();
void VI_GX_showStats();
void VI_GX_cleanUp();
void VI_GX_renderCpuFramebuffer();
void VI_GX_PreRetraceCallback(u32 retraceCnt);

#endif // __GX__

#endif

